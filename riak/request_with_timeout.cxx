#include <riak/request_with_timeout.hxx>

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
        response_handler& h,
        boost::asio::io_service& ios)
  : timeout_length_(timeout),
    timeout_(ios),
    request_data_(data),
    response_callback_(h),
    succeeded_(false),
    timed_out_(false)
{   }


void request_with_timeout::dispatch_via (transport& p)
{ 
    assert(not option_to_terminate_request_);
    auto on_response = std::bind(&request_with_timeout::on_response, shared_from_this(), _1, _2, _3);
    auto me = shared_from_this();
    option_to_terminate_request_ = p.deliver(me, on_response);
    
    timeout_.expires_from_now(boost::posix_time::milliseconds(timeout_length_.count()));
    auto on_timeout = std::bind(&request_with_timeout::on_timeout, shared_from_this(), _1);
    timeout_.async_wait(on_timeout);
}


void request_with_timeout::on_response (
        std::error_code error,
        size_t bytes_received,
        const std::string& raw_data)
{
    assert(not succeeded_);
    unique_lock<mutex> serialize(this->mutex_);
    succeeded_ = not (timed_out_ or error);
    if (succeeded_) {
        timeout_.cancel();
        if (response_callback_(std::error_code(), bytes_received, raw_data))
            option_to_terminate_request_->exercise();
    } else if (error != std::errc::operation_canceled) {
        timeout_.cancel();
        response_callback_(error, 0, raw_data);
        option_to_terminate_request_->exercise();
    }
}


void request_with_timeout::on_timeout (const boost::system::error_code& error)
{
    assert(not timed_out_);
    unique_lock<mutex> serialize(this->mutex_);
    timed_out_ = not error;
    if (timed_out_) {
        auto timeout_error = std::make_error_code(std::errc::timed_out);
        response_callback_(timeout_error, 0, "");
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
