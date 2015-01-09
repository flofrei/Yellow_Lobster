/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 *	 
 *	[DESCRIPTION]
 *	Implementing diffrent locking strategies in a unified user interface.
 *	The Code contains the following locks
 *	- OpenMP lock
 *	- STL C++11 mutex
 *	- TAS lock
 *	- TATAS lock
 *	- TAS expbo lock
 *	- FUTEX lock (Linux only)
 */

#ifndef LOCKS_HPP
#define LOCKS_HPP

#include <omp.h>
#include <mutex>
#include <vector>

#include "atomics.hpp"

#define USE_INTRINSICS

/****************************
 * 		OpenMP Lock 		*
 ****************************/
class omp_lock
{
public:
	omp_lock()  { omp_init_lock(&lock_);    }
	~omp_lock() { omp_destroy_lock(&lock_); }
	
	inline void lock()   { omp_set_lock(&lock_);   }
	inline void unlock() { omp_unset_lock(&lock_); }
private:
	omp_lock_t lock_;
};

/****************************
 * 		STL C++11 Lock 		*
 ****************************/
class STL_lock
{
public:
	inline void lock()   { lock_.lock();   }
	inline void unlock() { lock_.unlock(); }
private:
	std::mutex lock_;
};

/****************************
 * 			TAS lock 		*
 ****************************/
class TAS_lock
{
public:
	TAS_lock() : lock_(0) {}
	
	inline void lock()
	{
#ifdef USE_INTRINSICS
		while (__sync_lock_test_and_set(&lock_, 1)) do_nothing();
#else
		register unsigned char res = 1;
		while (res != 0) 
			asm __volatile__( "xchg %0,%1" : "+q" (res), "+m" (lock_) : : );
#endif
	}
	
	inline void unlock()
	{
#ifdef USE_INTRINSICS
		__sync_lock_release(&lock_);
#else
    	asm __volatile__("mfence" : : : "memory" );
        lock_ = 0;
#endif
	 }
private:
	volatile int lock_;
};


/****************************
 * 		TATAS lock 			*
 ****************************/
class TATAS_lock
{
public:
	TATAS_lock() : lock_(0) {}
	
	inline void lock()
	{
#ifdef USE_INTRINSICS
		while (__sync_lock_test_and_set(&lock_, 1))
			while (lock_ == 1) do_nothing();
#else
    	register unsigned char res = 1;
    	while (lock_ == 1) do_nothing();
    	while (res != 0)
    		asm __volatile__("xchg %0,%1" :"+q" (res), "+m" (lock_) : : );
#endif
	}
	
	inline void unlock()
	{
#ifdef USE_INTRINSICS
		__sync_lock_release(&lock_);
#else
    	asm __volatile__("mfence" : : : "memory");
        lock_ = 0;
#endif
	 }
private:
	volatile int lock_;
};

/****************************
 * 		TAS expbo 			*
 ****************************/
class TASexpbo_lock
{
public:
	TASexpbo_lock() : lock_(0) {}
	
	inline void lock()
	{
#ifdef USE_INTRINSICS
		int time = 1;
		while (__sync_lock_test_and_set(&lock_, 1))
	    {
			if(lock_ == 1)
			{
				time *= 2;
				for(int i = 0; i < time; ++i)
					do_nothing();
			}
		}
#else
		int time = 1;
    	register unsigned char res = 1;
    	while (res != 0)
    	{
    		asm __volatile__("xchg %0,%1" :"+q" (res), "+m" (lock_) : : "memory");
			{
				time *= 2;
				for(int i = 0; i < time; ++i)
					do_nothing();
			}
    	}
#endif
	}
	
	inline void unlock()
	{
#ifdef USE_INTRINSICS
		__sync_lock_release(&lock_);
#else
    	asm __volatile__("mfence" : : : "memory");
        lock_ = 0;
#endif
	 }
private:
	volatile int lock_;
};
 
// The following code only works on linux
// The code is inspired by http://locklessinc.com/articles/mutex_cv_futex/
#ifdef __linux__ 

#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>

#include "atomics.hpp"

inline long sys_futex(	void *addr1, int op, int val1, struct timespec *timeout, 
						void *addr2, int val3)
{
	return syscall(SYS_futex, addr1, op, val1, timeout, addr2, val3);
}

/****************************
 * 		FUTEX lock 			*
 ****************************/
class futex_lock
{
public:
	futex_lock(int local_spin_cnt = 100) 
		: lock_(0), local_spin_cnt_(local_spin_cnt) 
	{}

	inline void lock()
	{
		int lock_status = 0;
		
		// First we spin locally for a while and try to get the lock
		for(int i = 0; i < local_spin_cnt_; i++)
		{
			lock_status = atomic_cmpxchgl(&lock_, 0, 1);
			if(!lock_status) return;
			do_nothing();
		}

		// The lock is now contended
		if(lock_status == 1) 
			lock_status = atomic_xchgl(&lock_, 2);

		// We wait in the kernel until we get the lock
		while(lock_status)
		{
			sys_futex(&lock_, FUTEX_WAIT_PRIVATE, 2, NULL, NULL, 0);
			lock_status = atomic_xchgl(&lock_, 2);
		}
	}
	
	inline void unlock()
	{
		// If no one wants the lock just unlock it
		if(lock_ == 2)
			lock_ = 0;
		else if(atomic_xchgl(&lock_, 0) == 1) 
			return;

		// We spin locally for a while in the hope someone takes the lock
		for(int i = 0; i < 2*local_spin_cnt_; i++)
		{
			if (lock_ == 1)
				if (atomic_cmpxchgl(&lock_, 1, 2)) return;
			do_nothing();
		}
	
		// Wake someone up 
		sys_futex(&lock_, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
	}

private:
	/*
	 *	The lock variable lock_ can be either
	 *	0 = unlocked
	 *	1 = locked
	 *	2 = contended
	 */
	int lock_;
	int local_spin_cnt_;
};
#endif // __linux__

#endif // LOCKS_HPP
