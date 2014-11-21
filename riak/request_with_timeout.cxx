#include "request_with_timeout.hxx"
#include <riak/error.hxx>
#include <riak/utility/timer.hxx>
#include <riak/utility/timer_factory.hxx>

//=============================================================================
namespace riak {
//=============================================================================

using boost::mutex;
using boost::unique_lock;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


request_with_timeout::request_with_timeout (
		const std::string& data,
		std::chrono::milliseconds timeout,
		message::buffering_handler h,
		utility::timer_factory& timer_factory)
  : timeout_length_(timeout)
  , timer_(timer_factory.create())
  , response_callback_(h)
  , request_data_(data)
  , succeeded_(false)
  , timed_out_(false)
{	}


request_with_timeout::request_with_timeout (
		const std::string& data,
		std::chrono::milliseconds timeout,
		message::buffering_handler h,
		utility::timer_factory&& timer_factory)
  : timeout_length_(timeout)
  , timer_(timer_factory.create())
  , response_callback_(h)
  , request_data_(data)
  , succeeded_(false)
  , timed_out_(false)
{	}


request_with_timeout::~request_with_timeout ()
{	}


void request_with_timeout::dispatch_via (const transport::delivery_provider& deliver)
{
	// The request can only be sent once.
	assert(not terminate_request_ and not succeeded_ and not timed_out_);
	auto on_response = std::bind(&request_with_timeout::on_response, shared_from_this(), _1, _2, _3);
	terminate_request_ = deliver(request_data_, on_response);

	auto fail_request = std::bind(&request_with_timeout::on_timeout, shared_from_this(), _1);
	timer_->run_on_timeout(timeout_length_, fail_request);
}


void request_with_timeout::on_response (std::error_code error, size_t bytes_received, const std::string& raw_data)
{
	// Retain myself, for safety (strange external implementations may forget me
	// early, e.g. mocked transports)
	//
	auto self = shared_from_this();

	// The response should only arrive once, and exactly once.
	//
	assert(not succeeded_);
	unique_lock<mutex> serialize(this->mutex_);

	// Whatever happened, it constitutes activity. Stop the timeout timer.
	//
	timer_->cancel();

	if (this->is_live()) {
		bool error_condition = (timed_out_ or error);
		if (not error_condition) {
			if (response_callback_(std::error_code(), bytes_received, raw_data)) {
				(*terminate_request_)(false);
				succeeded_ = true;
				terminate_request_.reset();
			} else {
				auto fail_request = std::bind(&request_with_timeout::on_timeout, shared_from_this(), _1);
				timer_->run_on_timeout(timeout_length_, fail_request);
			}
		} else {
			(*terminate_request_)(true);

			// Timeout already satisfied the response callback.
			if (not timed_out_)
				response_callback_(error, 0, raw_data);

			terminate_request_.reset();
		}
	} else {
		// The response should only arrive once, and exactly once.
		//
		assert(timed_out_);
	}
}


void request_with_timeout::on_timeout (const std::error_code& error)
{
	// Retain myself, for safety (strange external implementations may forget me
	// early, e.g. mocked transports)
	//
	auto self = shared_from_this();

	assert(not timed_out_);
	unique_lock<mutex> serialize(this->mutex_);
	timed_out_ = not error;
	if (is_live() and timed_out_) {
		auto timeout_error = make_error_code(communication_failure::response_timeout);
		response_callback_(timeout_error, 0, "");
		terminate_request_.reset();
	} else {
		// Either canceled or raced with a last-minute response.
	}
}


bool request_with_timeout::is_live () const {
	return !! terminate_request_;
}

//=============================================================================
}   // namespace riak
//=============================================================================
