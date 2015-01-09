/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 *
 *	Serial comparision of the CPQ implementation with the TBB priority queue.
 */

#include <iostream>
#include <cassert>
#include <random>
#include <chrono>

#include <tbb/concurrent_priority_queue.h>
#include "CPQ.hpp"

typedef std::size_t test_t;

typedef CPQ<test_t, omp_lock, Linear_counter> CPQueue;

void test_serial(const std::size_t problem_size, const std::size_t init_size, 
				 const std::size_t seed);

bool queues_are_equal(CPQueue&, tbb::concurrent_priority_queue<test_t>&);

int main(int argc, char* argv[])
{	

	std::size_t problem_size = 1e6;
	std::size_t init_size = 1e4;

	std::size_t seed = std::chrono::system_clock::now().time_since_epoch().count();
	
	test_serial(problem_size, init_size, seed); 
	
	return 0;
}

// Perform a serial validation test (mixed insert/delete)
void test_serial(const std::size_t problem_size, const std::size_t init_size, 
				 const std::size_t seed) 
{
	std::cout << "Comparing serial inserts and deletes with TBB ... " << std::flush;

	CPQueue queue_CPQ;
	tbb::concurrent_priority_queue<test_t> queue_intel;
	
	std::default_random_engine rng(seed);
	
	test_t priority;
	
	// First insert some data into the queues
	for(size_t i = 0; i < init_size; ++i)
	{
		priority = rng();
		queue_CPQ.insert(priority, priority);
		queue_intel.push(priority);
	}

	test_t value_CPQ, value_intel;

	// Next insert or delete items with equal probability
	for(size_t i = 0; i < problem_size; ++i)
	{
		if(rng() % 2)
		{
			queue_CPQ.pop_front(value_CPQ);
			queue_intel.try_pop(value_intel);
		}
		else
		{
			priority = rng();
			queue_CPQ.insert(priority,priority);
			queue_intel.push(priority);
		}
	}
	
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
