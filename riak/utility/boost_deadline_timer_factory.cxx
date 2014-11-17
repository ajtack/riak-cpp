#include "boost_deadline_timer_factory.hxx"
#include <boost/asio/deadline_timer.hpp>
#include <riak/utility/timer.hxx>
#include <system_error>

//=============================================================================
namespace riak {
	namespace utility {
		namespace {
//=============================================================================

class wrapped_boost_deadline_timer
      : public timer
{
  public:
	wrapped_boost_deadline_timer (boost::asio::io_service& ios)
	  : physical_timer_(ios)
	{	}

	virtual void run_on_timeout (const std::chrono::milliseconds&, std::function<void(const std::error_code&)>);
	virtual std::size_t cancel ();

  private:
	boost::asio::deadline_timer physical_timer_;
};

//=============================================================================
		}   // namespace (anonymous)
//=============================================================================

boost_deadline_timer_factory::boost_deadline_timer_factory (boost::asio::io_service& ios)
  :	ios_(ios)
{	}


std::unique_ptr<timer> boost_deadline_timer_factory::create ()
{
	return std::unique_ptr<timer>(new wrapped_boost_deadline_timer(ios_));
}

//=============================================================================
		namespace {
//=============================================================================

/*!
 * Invokes callback with a std::error_code equivalent in meaning to that of the given
 * boost::system::error_code value. This keeps boost::asio details from leaking up
 * to an otherwise beautiful, standard API.
 */
void run_with_converted_error_code (
		const std::function<void(std::error_code)>& callback,
		const boost::system::error_code& original_error)
{
	if (! original_error) {
		callback(std::error_code());
	} else {
		assert(false);
	}
}


void wrapped_boost_deadline_timer::run_on_timeout (
		const std::chrono::milliseconds& milliseconds_to_expiry,
		std::function<void(const std::error_code&)> callback)
{
	using std::placeholders::_1;
	physical_timer_.expires_from_now(boost::posix_time::milliseconds(milliseconds_to_expiry.count()));
	auto wrapped_callback = std::bind(&run_with_converted_error_code, callback, _1);
	physical_timer_.async_wait(wrapped_callback);
}


std::size_t wrapped_boost_deadline_timer::cancel () {
	return physical_timer_.cancel();
}

//=============================================================================
		}   // namespace (anonymous)
	}   // namespace utility
}   // namespace riak
//=============================================================================
