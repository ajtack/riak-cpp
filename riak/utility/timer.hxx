#include <functional>
#include <system_error>
#pragma once
#ifdef _WIN32
#	include <boost/chrono.hpp>
	namespace std { namespace chrono = boost::chrono; }
#else
#	include <chrono>
#endif

//=============================================================================
namespace riak {
	namespace utility {
//=============================================================================

/*!
 * Provides basic time-based scheduling in a form that can be mocked for testing.
 */
class timer
{
  public:
	typedef std::function<void(const std::error_code&)> callback;

	virtual ~timer ()
	{	}

	/*!
	 * Schedules an operation to occur after the given timeout.
	 * \param c will be invoked without error if the timer expires, with
	 *     error if the timer is canceled, and otherwise not invoked at all.
	 */
	virtual
	void run_on_timeout (const std::chrono::milliseconds& timeout, callback c) = 0;

	/*!
	 * Remove all previously scheduled events from this timer. Note that any callback
	 * could have already been scheduled for execution at the point this is called.
	 *
	 * \return the number of pending operations (not yet triggered) that were canceled.
	 * Any callbacks that have already been fired (or scheduled for execution after
	 * a deadline passed) will not be included in this count, even if they haven't yet
	 * run to completion.
	 */
	virtual
	std::size_t cancel () = 0;
};

//=============================================================================
	}   // namespace utility
}   // namespace riak
//=============================================================================
