/**	
 *	CPQ, version 1.0  
 * 	(c) 2015 Michel Breyer, Florian Frei, Fabian Thüring ETH Zurich
 */

#ifndef NODE_HPP
#define NODE_HPP

#include "locks.hpp"

/* Tag:	-1 	: EMPTY
 		-2 	: AVAILABLE
 	    >0 	: thread id of owner
*/

const int EMPTY		= -1;
const int AVAILABLE = -2;

template<typename value_t, class lock_t>
class Node
{
public:

	/* Constructor */
	Node() 
		: value_(0.0), priority_(0), tag_(EMPTY), lock_()
	{}
		
	inline void init(value_t value, std::size_t priority, int pid)
	{
		value_ 		= value;
		priority_	= priority;
		tag_ 		= pid;
	}
	
	inline void lock() { lock_.lock(); }
	
	inline void unlock() { lock_.unlock(); }
	
	inline void swap(Node<value_t, lock_t>& N)
	{
		value_t tmp_value		 = value_;
		std::size_t tmp_priority = priority_;
		int tmp_tag			 	 = tag_;
		
		value_	  = N.value();
		priority_ = N.priority();
		tag_ 	  = N.tag();
		
		N.set_value(tmp_value);
		N.set_priority(tmp_priority);
		N.set_tag(tmp_tag);
	}
	
	inline void set_value(value_t value) { value_ = value; }
	inline void set_priority(std::size_t priority) { priority_ = priority; }
	inline void set_tag(int tag) { tag_ = tag; }
	
	inline std::size_t priority() const { return priority_; }
	inline value_t value() const { return value_; }
	inline int tag() const { return tag_; }

private:
	value_t value_;
	std::size_t priority_;
	int tag_;
	lock_t lock_;
};

#endif
