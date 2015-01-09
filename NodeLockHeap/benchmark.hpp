/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 *
 *  Insert only, delete only and mixed benchmarks for concurrent priority queues 
 *
 *	The problem size is fixed while the number of concurrent threads changes.
 *	The priorities are chosen randomly and a constant seed guarantees that the exact
 *	same sequence of operations is repeated.
 */

#ifndef BENCHMARK_HPP
#define BENCHMARK_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <random>
#include <chrono>
#include <queue>
#include <omp.h>
#include <cmath>
#include <string>

#include "CPQ.hpp"
#include "tbb/concurrent_priority_queue.h"
#include "timer.hpp"

template <class value_t, class lock_t, class counter_t, class ostream_t = std::ostream >
void benchmark_insert_operations(const std::size_t problem_size, const std::size_t init_size,
								 const std::size_t nreps, const std::size_t seed, 
								 const std::size_t max_nthreads, ostream_t& out = std::cout);

template <class value_t, class lock_t, class counter_t, class ostream_t = std::ostream >
void benchmark_delete_operations(const std::size_t problem_size, const std::size_t init_size, 
								 const std::size_t nreps, const std::size_t seed, 
								 const std::size_t max_nthreads, ostream_t& out = std::cout);

template <class value_t, class lock_t, class counter_t, class ostream_t = std::ostream >
void benchmark_mixed_operations(const std::size_t problem_size, const std::size_t init_size, 
								const std::size_t nreps, const std::size_t seed, 
								const std::size_t max_nthreads, ostream_t& out = std::cout);

/****************************
 * 			CPQ 	 		*
 ****************************/
template< 	class value_t,  class lock_t = omp_lock, 
			class counter_t = Bit_reversed_counter> 
class queue_CPQ
{
public:
	inline void push(value_t val, std::size_t priority) { queue_.insert(val, priority); }
	inline bool pop(value_t& val) { return queue_.pop_front(val); }
private:
	CPQ<value_t,lock_t,counter_t> queue_;
};

/****************************
 * 		Intel Queue			*
 ****************************/
template< 	class value_t,  class lock_t = omp_lock, 
			class counter_t = Bit_reversed_counter> 
class queue_Intel
{
public:
	inline void push(value_t val, std::size_t priority) { queue_.push(priority); }
	inline bool pop(value_t& val) { return queue_.try_pop(val); }
private:
	tbb::concurrent_priority_queue<std::size_t> queue_;
};

/****************************
 * 			STL Queue 		*
 ****************************/
template<	class value_t,  class lock_t, 
		 	class counter_t = Bit_reversed_counter> 
class queue_STL
{
public:
	inline void push(value_t val, std::size_t priority) 
	{ 
		lock_.lock();
		queue_.push(priority); 
		lock_.unlock();
	}
	
	inline bool pop(value_t& val) 
	{ 
		lock_.lock();
		if(queue_.empty())
		{	
			lock_.unlock();
			return false;
		}
		else
		{
			val = queue_.top();
			queue_.pop();
			lock_.unlock();
			return true;
		}
	}
private:
	STL_lock lock_;
	std::priority_queue<std::size_t> queue_;
};

#endif // BENCHMARK_HPP
