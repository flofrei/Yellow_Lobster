/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 *
 *	Provide a Timer class
 */

#ifndef TIMER_HPP
#define TIMER_HPP

#include <sys/time.h>
#include <iostream>

class Timer
{
public:
	Timer()
		: t_start_(0.0), tic_was_called(false)
	{
		gettimeofday(&t_, 0);
	}

	void tic()
	{
		gettimeofday(&t_, 0);
		t_start_ = t_.tv_sec+(t_.tv_usec/1000000.0);
		tic_was_called=true;
	}

	double toc()
	{
		if(!tic_was_called)
			std::cerr 	<<	"*** Warning *** : Calling toc() without previously "
						<<	"calling tic()" << std::endl;
		gettimeofday(&t_, 0);
		return t_.tv_sec+(t_.tv_usec/1000000.0) - t_start_;
	}
private:
	double t_start_;
	bool tic_was_called;
	struct timeval t_;
};

#endif
