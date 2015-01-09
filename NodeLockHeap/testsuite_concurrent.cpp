/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 *
 *	This file provides various tests to check the correctness of the CPQ implementation
 */

#include <iostream>
#include <random>
#include <chrono>
#include <cassert>
#include <omp.h>

#include <tbb/concurrent_priority_queue.h>
#include "CPQ.hpp"
#include "locks.hpp"

typedef std::size_t test_t;

typedef CPQ<test_t, omp_lock, Bit_reversed_counter> CPQueue;

void compare_concurrent_insert_with_intel(const std::size_t test_size, const std::size_t seed, 
										  const std::size_t nthreads);
void compare_concurrent_delete_with_intel(const std::size_t problem_size, const std::size_t seed,
	 									  const std::size_t nthreads);
void verify_heap_properties_insert(const std::size_t problem_size, const std::size_t seed,
								   const std::size_t nthreads);
void verify_heap_properties_mixed(const std::size_t problem_size, const std::size_t initial_size, 
								  const std::size_t seed, const std::size_t nthreads);

int main(int argc, char* argv[])
{	

	std::size_t problem_size = 1e5;
	std::size_t initial_size = 1e5;
	std::size_t nthreads = 8;

	std::size_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	
	std::cout << "Noperations:\t" << problem_size << std::endl;
	std::cout << "Nthreads:\t" << nthreads << std::endl;
	std::cout << "Seed:\t\t" << seed << std::endl << std::endl;
	
	compare_concurrent_insert_with_intel(problem_size, seed, nthreads);
	compare_concurrent_delete_with_intel(problem_size, seed, nthreads);
	
	verify_heap_properties_insert(problem_size, seed, nthreads);
	verify_heap_properties_mixed(problem_size, initial_size, seed, nthreads);
	
	return 0;
}

bool queues_are_equal(CPQueue& queue_CPQ, tbb::concurrent_priority_queue<test_t>& queue_intel);

void compare_concurrent_insert_with_intel(const std::size_t problem_size, const std::size_t seed,
										  const std::size_t nthreads)
{	
	std::cout << "Comparing concurrent insert with TBB ... " << std::flush;
	
	CPQueue queue_CPQ;
	tbb::concurrent_priority_queue<test_t> queue_intel;
	
	std::default_random_engine rng;
	
	// Insert items to the queue in parallel
	#pragma omp parallel private(rng) shared(queue_CPQ, queue_intel) num_threads(nthreads)
	{
		rng.seed(seed + omp_get_thread_num());
	
		#pragma omp for
		for (std::size_t i=0; i<problem_size; ++i)
		{
			test_t priority = rng();
			queue_CPQ.insert(priority, priority);
			queue_intel.push(priority);
		} 
	}
	
	// Then read data out in serial and compare values TBB's implementation
	if(queues_are_equal(queue_CPQ, queue_intel))
		std::cout << "PASSED" << std::endl;
	else
		std::cout << "FAILED" << std::endl;
}

void compare_concurrent_delete_with_intel(const std::size_t problem_size, const std::size_t seed, 
										  const std::size_t nthreads)
{	
	std::cout << "Comparing concurrent delete with TBB ... " << std::flush;
	
	CPQueue queue_CPQ;
	tbb::concurrent_priority_queue<test_t> queue_intel;
	
	std::default_random_engine rng(seed);
	
	// First insert some random elements
	test_t priority;
	for(size_t i = 0; i < problem_size; ++i)
	{
		priority = rng();
		queue_CPQ.insert(priority, priority);
		queue_intel.push(priority);
	}
	
	// Then concurrently remove a part of the elements
	const std::size_t nbr_elements_to_remove = problem_size / 2;	

	test_t value_CPQ, value_intel;

	#pragma omp parallel for private(value_CPQ, value_intel) shared(queue_CPQ, queue_intel) num_threads(nthreads)
	for(std::size_t i = 0; i < nbr_elements_to_remove; ++i)
	{
		queue_CPQ.pop_front(value_CPQ);
		queue_intel.try_pop(value_intel);
	}

	// And check if the remaining items are the same
	if(queues_are_equal(queue_CPQ, queue_intel))
		std::cout << "PASSED" << std::endl;
	else
		std::cout << "FAILED" << std::endl;
}

bool queues_are_equal(CPQueue& queue_CPQ, tbb::concurrent_priority_queue<test_t>& queue_intel)
{
	assert(queue_CPQ.size() == queue_CPQ.size());
	
	bool are_equal = true;
	test_t value_CPQ(0), value_intel(0);
	
	while(!queue_CPQ.empty())
	{
		queue_CPQ.pop_front(value_CPQ);
		queue_intel.try_pop(value_intel);
		
		if(value_CPQ != value_intel)
		{
			std::cout << "\nERROR: \n" 
					  << " :\n\tvalue_CPQ \t " << value_CPQ
					  << " \n\tvalue_intel\t " << value_intel << std::endl;
			are_equal = false;
		}
	}
	
	return are_equal;
}

// This function verifies if the value returned by the delete routine is the
// greatest of all the items stored in the queue.
// This assumes the values are equal to the priorities
bool verifies_heap_properties(CPQueue& queue)
{
	bool properties_verified = true;
	
	test_t value, previous_value;
	
	queue.pop_front(previous_value);
	
	while(!queue.empty())
	{
		queue.pop_front(value);
		
		if (value > previous_value) 
		{
			std::cout << previous_value << '\t' << value << std::endl;
			properties_verified = false;
		}
		
		previous_value = value;
	}
	
	return properties_verified;
}

void verify_heap_properties_insert(const std::size_t problem_size, const std::size_t seed,
								   const std::size_t nthreads)
{
	std::cout << "Testing PQ properties after concurrent inserts ... " << std::flush;
	
	CPQueue queue;
	std::default_random_engine rng;
	
	#pragma omp parallel private(rng) shared(queue) num_threads(nthreads)
	{
		rng.seed(seed + omp_get_thread_num());
		
		#pragma omp for
		for (std::size_t i=0; i<problem_size; ++i)
		{
			test_t priority = rng();
			queue.insert(priority, priority);
		} 
	}
	
	if(verifies_heap_properties(queue))
		std::cout << "PASSED" << std::endl;
	else
		std::cout << "FAILED" << std::endl;
}

void verify_heap_properties_mixed(const std::size_t problem_size, const std::size_t initial_size,
								  const std::size_t seed, const std::size_t nthreads)
{
	std::cout << "Testing PQ properties after concurrent inserts and deletes ... " << std::flush;
	
	CPQueue queue;
	std::default_random_engine rng(seed);
	
	for (std::size_t i=0; i<initial_size; ++i)
	{
		test_t priority = rng();
		queue.insert(priority, priority);
	}
	
	#pragma omp parallel private(rng) shared(queue) num_threads(nthreads)
	{
		rng.seed(seed + omp_get_thread_num()+1);
		
		test_t priority, value;
		
		#pragma omp for	
		for (std::size_t i=0; i<problem_size; ++i)
		{
			if (rng() % 2)
			{
				priority = rng();
				queue.insert(priority, priority);
			}
			else
				queue.pop_front(value);
		} 
	}
	
	if (verifies_heap_properties(queue))
		std::cout << "PASSED" << std::endl;
	else
		std::cout << "FAILED" << std::endl;
}

