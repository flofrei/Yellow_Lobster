/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 *
 *	Implementing various atomic operations
 */

#ifndef ATOMICS_HPP
#define ATOMICS_HPP

inline void atomic_increment(volatile int* ptr)
{
	asm __volatile__(	"lock; addl %1, %0"
		    		 	: "+m" (*ptr)
		     		 	: "ir" (1)
		     		 	: "memory");
}

inline void atomic_decrement(volatile int* ptr)
{
	asm __volatile__(	"lock; subl %1, %0"
		    		 	: "+m" (*ptr)
		     		 	: "ir" (1)
		     		 	: "memory");
}

inline unsigned atomic_cmpxchgl(int* ptr, int old_val, int new_val )
{ 
  	volatile int * __ptr = (volatile int *)(ptr);
  	int ret_val; 
  	asm __volatile__( 	"lock; cmpxchgl %2,%1"
    					: "=a" (ret_val), "+m" (*__ptr)
    					: "r" (new_val), "0" (old_val)
    					: "memory");
	return ret_val;
}

inline unsigned atomic_xchgl(int *ptr, int val)
{
	asm __volatile__(	"xchgl %0,%1"
						:"=r" (val)
						:"m" (*(volatile int *)ptr), "0" (val)
						:"memory");
	return val;
}

inline void do_nothing() 
{ 
	asm __volatile__("nop");
}

#endif // ATOMICS_HPP
