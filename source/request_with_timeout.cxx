#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <request_with_timeout.hxx>

//=============================================================================
namespace riak {
//=============================================================================

namespace system = boost::system;
namespace asio = boost::asio;
using boost::mutex;
using boost::unique_lock;
using std::placeholders::_1;
using std::placeholders::_2;

request_with_timeout::request_with_timeout (
        const std::string& data,
        std::chrono::milliseconds timeout,
        boost::asio::ip::tcp::socket& s,
        response_handler& h,
        boost::asio::io_service& ios)
  : socket_(s),
    timeout_length_(timeout),
    timeout_(ios),
    request_data_(data),
    response_callback_(h),
    succeeded_(false),
    timed_out_(false)
{   }


void request_with_timeout::dispatch ()
{ 
    auto on_write = std::bind(&request_with_timeout::on_write, shared_from_this(), _1, _2);
    auto on_timeout = std::bind(&request_with_timeout::on_timeout, shared_from_this(), _1);
    timeout_.expires_from_now(boost::posix_time::milliseconds(timeout_length_.count()));
    asio::async_write(socket_, asio::buffer(request_data_), on_write);
    timeout_.async_wait(on_timeout);
}


void request_with_timeout::on_response (const system::error_code& error, size_t)
{
    assert(not succeeded_);
    unique_lock<mutex> serialize(this->mutex_);
    succeeded_ = not (timed_out_ or error);
    if (succeeded_) {
        timeout_.cancel();
        response_callback_(system::error_code(), response_data_);
    } else if (error != asio::error::operation_aborted) {
        timeout_.cancel();
        response_callback_(error, response_data_);
    }
}


void request_with_timeout::on_timeout (const system::error_code& error)
{
    assert(not timed_out_);
    unique_lock<mutex> serialize(this->mutex_);
    timed_out_ = not error;
    if (timed_out_) {
        using namespace system;
        auto timeout_error = error_code(errc::timed_out, get_generic_category());
        response_callback_(timeout_error, response_data_);

        // We want to be sure we don't receive any stale replies the next time we use this.
        auto last_target = socket_.remote_endpoint();
        socket_.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket_.close();
        socket_.connect(last_target);
    }
}

//=============================================================================
    namespace {
//=============================================================================

typedef asio::buffers_iterator<asio::streambuf::const_buffers_type> iterator;

/*!
 * Intended to delay reception until the response constitutes a complete Riak command of the form
 * | Rest-of-Message Length (32 bits) | Message Code (8 bits) | Message Body |.
 */
std::pair<iterator, bool> response_complete (const iterator begin, const iterator end)
{
    if (end - begin >= sizeof(uint32_t) + sizeof(char)) {
        uint32_t encoded_length = *reinterpret_cast<const uint32_t*>(&*begin);
        uint32_t data_length = ntohl(encoded_length);
        
        if (end - (begin + sizeof(encoded_length)) >= data_length) {
            iterator new_beginning = begin + sizeof(encoded_length) + data_length;   // day breaks!
            return std::make_pair(new_beginning, true);
        } else {
            return std::make_pair(begin, false);
        }
    } else {
        return std::make_pair(begin, false);
    }
}

//=============================================================================
    }   // namespace anonymous
//=============================================================================

void request_with_timeout::on_write (const system::error_code& error, size_t)
{
    unique_lock<mutex> serialize(this->mutex_);
    
    if (not (timed_out_ or error)) {
        auto response_handler = std::bind(&request_with_timeout::on_response, shared_from_this(), _1, _2);
        asio::async_read_until(socket_, response_data_, &response_complete, response_handler);
    } else if (error != asio::error::operation_aborted) {
        timeout_.cancel();
        response_callback_(error, response_data_);
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
