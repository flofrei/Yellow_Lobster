/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 *
 *	Concurrent Priority Queue
 */

#ifndef CPQ_HPP
#define CPQ_HPP

#include <iostream>
#include <iomanip>
#include <vector>

#include <omp.h>

#include "bit_reversed_counter.hpp"
#include "Node.hpp"
#include "locks.hpp"
#include "atomics.hpp"

template< class value_t,  class lock_t = omp_lock, 
		  class counter_t = Bit_reversed_counter>
class CPQ
{	
public:
		 
	/* Constructor */
	CPQ() 
		: size_(), thread_count_(0)
	{
		// Insert dummy element in order to have a one based array
		heap_.push_back(Node<value_t, lock_t>());
	}
			
	/** 
	 *	insert: Inserts an element (value, priority) into the priority queue 
	 */
	void insert(value_t value, std::size_t priority)
	{	
		heap_lock.lock();
		int pid = omp_get_thread_num();
		std::size_t child = size_.increment();
		
		// If the current level is full allocate memory for the next one.
		// We first have to make sure that no other thread is currently in the
		// queue i.e all locks are unlocked.
		if(size() == heap_.size())
 		{
 			// Wait until all other threads left
 			while(thread_count_ != 0) do_nothing();

 			for(std::size_t i = 0; i < size_.high_bit(); ++i)
 			{
 				heap_.push_back(Node<value_t, lock_t>());
 			}
 		}
		
		// Atomically increment the thread count
		atomic_increment(&thread_count_);
		
		heap_[child].lock();
		
		heap_[child].init(value, priority, pid);
		heap_lock.unlock();	
		
		heap_[child].unlock();
		
		std::size_t parent, old_child = 0;
		
		while(child > ROOT)
		{
			parent = child >> 1;
			
			heap_[parent].lock();
			heap_[child].lock();
			
			old_child = child;
			
			if(heap_[parent].tag() == AVAILABLE &&  heap_[child].tag() == pid)
			{
				if (heap_[child].priority() > heap_[parent].priority())
				{
					heap_[child].swap(heap_[parent]);
					child = parent;
				}
				else
				{
					heap_[child].set_tag(AVAILABLE);
					child = 0;
				}
			}
			else if (heap_[parent].tag() == EMPTY)
				child = 0;
			else if (heap_[child].tag() != pid)
				child = parent;
			
			heap_[old_child].unlock();
			heap_[parent].unlock();
		}
		
		if (child == ROOT)
		{
			heap_[ROOT].lock();
			if (heap_[ROOT].tag() == pid)
				heap_[ROOT].set_tag(AVAILABLE);
			heap_[ROOT].unlock();
		}
		
		// We are done decrement thread count
		atomic_decrement(&thread_count_);
	}
	

	/** 
	 *	pop_front: 	Assigns the value of the first element in the queue to
	 *				the parameter value. Returns false if the assignement failed.
	 */
	bool pop_front(value_t& value)
	{	
		heap_lock.lock();
		
		// Atomically increments the thread count
		atomic_increment(&thread_count_);
		
		if (empty())
		{
			heap_lock.unlock();
			atomic_decrement(&thread_count_);
			return false;
		}
		
		std::size_t bottom = size_.decrement();
		
		heap_[bottom].lock();
		heap_lock.unlock();
		
		value_t value_bottom = heap_[bottom].value();
		std::size_t priority_bottom = heap_[bottom].priority();
		heap_[bottom].set_tag(EMPTY);
		
		heap_[bottom].unlock();
		
		heap_[ROOT].lock();
		
		// if there is only one entry in the heap return
		if (heap_[ROOT].tag() == EMPTY)
		{		
			value = heap_[ROOT].value();
			heap_[ROOT].unlock();
			
			atomic_decrement(&thread_count_);
			return true;
		}
		
		// else insert the bottom element at the top and let it sink
		value = heap_[ROOT].value();
		
		heap_[ROOT].init(value_bottom, priority_bottom, AVAILABLE);
		
		// Restore heap properties
		std::size_t parent = ROOT;
		std::size_t right, left, child;
		
		while(2 * parent <= heap_.size()-1)
		{
			left = parent << 1;
			right = left + 1;
			
			heap_[left].lock();
			heap_[right].lock();
			
			if (heap_[left].tag() == EMPTY)
			{
				heap_[right].unlock();
				heap_[left].unlock();
				break;
			}
			else if (heap_[right].tag() == EMPTY || 
					 heap_[left].priority() > heap_[right].priority())
			{
				heap_[right].unlock();
				child = left;
			}
			else
			{
				heap_[left].unlock();
				child = right;
			}

			if (heap_[child].priority() > heap_[parent].priority())
			{
				heap_[child].swap(heap_[parent]);
				heap_[parent].unlock();
				parent = child;
			}
			else
			{
				heap_[child].unlock();
				break;
			}

		}
		heap_[parent].unlock();
		
		// We are done decrement thread count
		atomic_decrement(&thread_count_);
		return true;
	}
	
	inline bool empty() const { return size_.counter() < 1 ; }
	inline std::size_t size() const { return size_.counter(); }
	
private:
	std::vector< Node<value_t, lock_t> > heap_;
	counter_t size_;
	lock_t heap_lock;
	
	volatile int thread_count_;
	
	static const std::size_t ROOT = 1;
};

#endif // CPQ_HPP
