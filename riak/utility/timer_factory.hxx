#pragma once
#include <memory>

namespace riak {
	namespace utility { class timer; }
}

//=============================================================================
namespace riak {
	namespace utility {
//=============================================================================

class timer_factory
{
  public:
	virtual ~timer_factory ()
	{	}

	/*!
	 * \return a new, unique timer object with nothing scheduled.
	 */
	virtual std::unique_ptr<timer> create () = 0;
};

//=============================================================================
	}   // namespace utility
}   // namespace riak
//=============================================================================
