#pragma once
#include <boost/thread/mutex.hpp>
#include <boost/optional.hpp>
#include <memory>
#include <riak/message.hxx>
#include <riak/transport.hxx>
#include <system_error>

#ifdef _WIN32
#	include <boost/chrono.hpp>
	namespace std { namespace chrono = boost::chrono; }
#else
#	include <chrono>
#endif

namespace riak {
	namespace utility {
		class timer;
		class timer_factory;
	}
}

//=============================================================================
namespace riak {
//=============================================================================

class request_with_timeout
    : public std::enable_shared_from_this<request_with_timeout>
{
  public:
	/*!
	 * Packages a task, but does not send it or begin timeout calculations.
	 * \param timeout is in milliseconds. It determines the maximum length of any radio silence
	 *        from the server.
	 */
	request_with_timeout (
			const std::string& data,
			std::chrono::milliseconds timeout,
			message::buffering_handler h,
			utility::timer_factory& timer_factory);

	request_with_timeout (
			const std::string& data,
			std::chrono::milliseconds timeout,
			message::buffering_handler h,
			utility::timer_factory&& timer_factory);

	~request_with_timeout ();
	
	/*!
	 * Sends the request and begins listening for a response asynchronously. Timeout countdown starts now.
	 * \pre There must exist a shared pointer to this request at the time of dispatch. It may be
	 *	 reset as soon as the request is dispatched.
	 */
	void dispatch_via (const transport::delivery_provider& p);

  private:
	void on_response (std::error_code, std::size_t, const std::string&);
	void on_timeout (const std::error_code&);

	bool is_live () const;
	  
	mutable boost::mutex mutex_;
	std::chrono::milliseconds timeout_length_;
	std::unique_ptr<utility::timer> timer_;
	message::buffering_handler response_callback_;
	boost::optional<transport::option_to_terminate_request> terminate_request_;
	const std::string request_data_;
	
	bool succeeded_;
	bool timed_out_;
};

//=============================================================================
}   // namespace riak
//=============================================================================
