/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 *
 *	Bit reversed counter for concurrent priority queues 
 *	The following code was adapted from Hunt et al., 1996
 */

#ifndef BIT_REVERSED_COUNTER_HPP
#define BIT_REVERSED_COUNTER_HPP

class Bit_reversed_counter
{
public:
	Bit_reversed_counter()
		: counter_(0), reverse_(0)
	{}
	
	inline int increment()
	{
		if (counter_++ == 0)
		{
			reverse_ = high_bit_ = 1;
			return reverse_;
		}
		
		std::size_t bit = high_bit_ >> 1;
		while (bit)
		{
			reverse_ ^= bit;
			if (reverse_ & bit) break;
			bit >>= 1;
		}
		
		if (!bit)
			reverse_ = high_bit_ <<= 1;
		
		return reverse_;
	}
	
	inline int decrement()
	{
		std::size_t reverse_before_decrement = reverse_;
		
		counter_--;
		
		std::size_t bit = high_bit_ >> 1;
		while (bit)
		{
			reverse_ ^= bit;
			if (!(reverse_ & bit)) break;
			bit >>= 1;
		}
		
		if (!bit)
		{
			reverse_ = counter_;
			high_bit_ >>= 1; 
		}
		
		return reverse_before_decrement;
	}
		
	inline std::size_t counter() const { return counter_; }
	inline std::size_t high_bit() const { return high_bit_; }

private:
	std::size_t counter_;
	std::size_t reverse_;
	std::size_t high_bit_;
};

class Linear_counter
{
public:
    Linear_counter()
    : counter_(0), high_bit_(1)
    {}
    
    inline int increment()
    {
        if(counter_ == high_bit_)
            high_bit_ <<= 1;
      
		return ++counter_;
	}
    
    inline int decrement()
    {
        if(counter_ == high_bit_)
			high_bit_ >>= 1;
		
		return counter_--;
    }
	
	inline std::size_t counter() const { return counter_; }
	inline std::size_t high_bit() const { return high_bit_; }
    
private:
    std::size_t counter_;
    std::size_t high_bit_;
};

#endif // BIT_REVERSED_COUNTER_HPP
