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

#include "benchmark.hpp"

int main(int argc , char* argv[])
{
	std::size_t max_nthreads = 7;
	std::size_t nreps = 2;
	
	std::size_t problem_size = 1 << 15;
	std::size_t init_size = 1 << 17;
	
	std::size_t seed = 1;

	std::ofstream fout_insert;
	std::ofstream fout_delete;
	std::ofstream fout_mixed;
	
	std::string output = "output/";
		
#ifdef _CPQ
	fout_insert.open(output+"insert_omp.dat");
	fout_delete.open(output+"delete_omp.dat");
	fout_mixed.open(output+"mixed_omp.dat");
#elif defined(_Intel)
	fout_insert.open(output+"insert_Intel.dat");
	fout_delete.open(output+"delete_Intel.dat");
	fout_mixed.open(output+"mixed_Intel.dat");
#elif defined(_STL)
	fout_insert.open(output+"insert_STL.dat");
	fout_delete.open(output+"delete_STL.dat");
	fout_mixed.open(output+"mixed_STL.dat");
#endif	

	 benchmark_insert_operations<std::size_t, omp_lock, Bit_reversed_counter>
	 	(problem_size, init_size, nreps, seed, max_nthreads, fout_insert);
	
	 benchmark_delete_operations<std::size_t, omp_lock, Bit_reversed_counter>
	 	(problem_size, init_size, nreps, seed, max_nthreads, fout_delete);
	
	  benchmark_mixed_operations<std::size_t, omp_lock, Bit_reversed_counter>
	  	(problem_size, init_size, nreps, seed, max_nthreads, fout_mixed );
	 
	fout_insert.close();
	fout_delete.close();
	fout_mixed.close();
	
	return 0;	
}

/****************************/
/*			Insert 			*/
/****************************/
template <class value_t, class lock_t, class counter_t, class ostream_t>
void benchmark_insert_operations(const std::size_t problem_size, const std::size_t init_size,
								 const std::size_t nreps, const std::size_t seed, 
								 const std::size_t max_nthreads, ostream_t& out)
{
	out << "Problem size:\t" << problem_size << std::endl;
	out << "Init size:\t" << init_size << std::endl;
	out << "Repetitions:\t" << nreps << std::endl;
	
	for (std::size_t nthreads=1; nthreads <= max_nthreads; nthreads+=2)
	{
		double sum_time = 0;
		double sum_time2 = 0;

		Timer timer;

		for (std::size_t n=0; n<nreps; ++n)
		{
			
#ifdef _CPQ
			queue_CPQ<value_t, lock_t, counter_t> queue;
#elif defined(_Intel)
			queue_Intel<value_t, lock_t, counter_t> queue;
#elif defined(_STL)
			queue_STL<value_t, lock_t, counter_t> queue;
#endif
			
			std::default_random_engine rng(seed);

			for (std::size_t i=0; i<init_size; ++i)
			{
				std::size_t priority = rng();
				queue.push(priority, priority);
			}

			timer.tic();

			#pragma omp parallel private(rng) shared(queue) num_threads(nthreads)
			{
				rng.seed(seed + omp_get_thread_num()+1);
				std::size_t priority;

				#pragma omp for
				for (std::size_t i=0; i<problem_size; ++i)
				{
					priority = rng();
					queue.push(priority, priority);
				}
			}

			double elapsed_time = timer.toc();
			sum_time += elapsed_time;
			sum_time2 += elapsed_time*elapsed_time;
		}

		double mean_time = sum_time / nreps;
		double sigma_time = std::sqrt(1./(nreps-1)*(sum_time2/nreps - mean_time*mean_time));

		out.precision(8);
		out << std::fixed;
		out << std::right << std::setw(20) << nthreads;
		out << std::right << std::setw(20) << mean_time;
		out << std::right << std::setw(20) << sigma_time << std::endl;
	}
}
	
/****************************/
/*			Delete 			*/
/****************************/	
template <class value_t, class lock_t, class counter_t, class ostream_t>
void benchmark_delete_operations(const std::size_t problem_size, const std::size_t init_size,
								 const std::size_t nreps, const std::size_t seed, 
								 const std::size_t max_nthreads, ostream_t& out)
{
	
	out << "Problem size:\t" << problem_size << std::endl;
	out << "Init size:\t" << init_size << std::endl;
	out << "Repetitions:\t" << nreps << std::endl;
	
	for (std::size_t nthreads=1; nthreads <= max_nthreads; nthreads+=2)
	{

		double sum_time = 0;
		double sum_time2 = 0;

		Timer timer;

		for (std::size_t n=0; n<nreps; ++n)
		{
			
#ifdef _CPQ
			queue_CPQ<value_t, lock_t, counter_t> queue;
#elif defined(_Intel)
			queue_Intel<value_t, lock_t, counter_t> queue;
#elif defined(_STL)
			queue_STL<value_t, lock_t, counter_t> queue;
#endif
			

			std::default_random_engine rng(seed);

			for (std::size_t i=0; i<init_size; ++i)
			{
				std::size_t priority = rng();
				queue.push(priority, priority);
			}

			timer.tic();

			#pragma omp parallel private(rng) shared(queue) num_threads(nthreads)
			{
				rng.seed(seed + omp_get_thread_num()+1);
				std::size_t priority;

				#pragma omp for
				for (std::size_t i=0; i<problem_size; ++i)
				{
					priority = rng();
					queue.pop(priority);
				}
			}

			double elapsed_time = timer.toc();
			sum_time += elapsed_time;
			sum_time2 += elapsed_time*elapsed_time;
		}

		double mean_time = sum_time / nreps;
		double sigma_time = std::sqrt(1./(nreps-1)*(sum_time2/nreps - mean_time*mean_time));

		out.precision(8);
		out << std::fixed;
		out << std::right << std::setw(20) << nthreads;
		out << std::right << std::setw(20) << mean_time;
		out << std::right << std::setw(20) << sigma_time << std::endl;
	}
}

/****************************/
/*			Mixed 			*/
/****************************/
template <class value_t, class lock_t, class counter_t, class ostream_t>
void benchmark_mixed_operations(const std::size_t problem_size, const std::size_t init_size, 
								const std::size_t nreps, const std::size_t seed, 
								const std::size_t max_nthreads, ostream_t& out)
{
	out << "Problem size:\t" << problem_size << std::endl;
	out << "Init size:\t" << init_size << std::endl;
	out << "Repetitions:\t" << nreps << std::endl;
	
	for (std::size_t nthreads=1; nthreads <= max_nthreads; nthreads+=2)
	{
		double sum_time = 0;  
		double sum_time2 = 0;
	
		Timer timer;
	
		for (std::size_t n=0; n<nreps; ++n)
		{

#ifdef _CPQ
			queue_CPQ<value_t, lock_t, counter_t> queue;
#elif defined(_Intel)
			queue_Intel<value_t, lock_t, counter_t> queue;
#elif defined(_STL)
			queue_STL<value_t, lock_t, counter_t> queue;
#endif
	
			std::default_random_engine rng(seed);
	
			for (std::size_t i=0; i<init_size; ++i)
			{
				std::size_t priority = rng();
				queue.push(priority, priority);
			}
	
			timer.tic();

			#pragma omp parallel private(rng) shared(queue) num_threads(nthreads)
			{
				rng.seed(seed + omp_get_thread_num()+1);
				std::size_t priority, value;
		
				#pragma omp for	
				for (std::size_t i=0; i<problem_size; ++i)
				{
					if (rng() % 2)
					{
						priority = rng();
						queue.push(priority, priority);
					}
					else
						queue.pop(value);
				} 
			}
	
			double elapsed_time = timer.toc();
			sum_time += elapsed_time;
			sum_time2 += elapsed_time*elapsed_time;
		}
	
		double mean_time = sum_time / nreps;
		double sigma_time = std::sqrt(1./(nreps-1)*(sum_time2/nreps - mean_time*mean_time));
	
 		out.precision(8);
		out << std::fixed;
		out << std::right 	<< std::setw(20) << nthreads
					 		<< std::setw(20) << mean_time
							<< std::setw(20) << sigma_time << std::endl;
	}
}

