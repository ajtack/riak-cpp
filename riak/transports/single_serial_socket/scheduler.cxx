#include "socket.hxx"
#include "resolver.hxx"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>
#include <riak/transport.hxx>
#include <riak/transports/single_serial_socket/scheduler.hxx>
#include <system_error>
#include <memory>
#include <iostream>

namespace asio = boost::asio;

//=============================================================================
namespace riak {
    namespace transport {
        namespace single_serial_socket {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;

scheduler::scheduler (
        const std::string& node_address,
        uint16_t port,
        boost::asio::io_service& ios,
        std::unique_ptr<socket> s,
        const std::shared_ptr<resolver>& resolver)
  : target_(node_address, boost::lexical_cast<std::string>(port))
  , ios_(ios)
  , socket_(std::move(s))
  , resolver_(resolver)
  , shutting_down_(false)
{
    connect_socket();
}


scheduler::~scheduler ()
{
    boost::unique_lock<boost::mutex> serialize(mutex_);
    
    // Prevent new requests from entering, so we don't race over the pending_requests_ list.
    shutting_down_ = true;
    
    // Report the shutdown to all clients.
    for (auto entry = pending_requests_.begin(); entry != pending_requests_.end(); ++entry) {
        auto pending_request = *entry;
        auto response_handler = pending_request->second;
        response_handler(std::make_error_code(std::errc::network_reset), 0, "");
    }
    
    while (not pending_requests_.empty())
        request_dequeued_.wait(serialize);
    
    // Requests may have been made active while we were cleaning, so this has to be last!
    socket_->cancel();
    while (active_request_)
        active_request_finished_.wait(serialize);
    
    using boost::asio::ip::tcp;
    socket_->shutdown(tcp::socket::shutdown_both);
    socket_->close();
}


transport::option_to_terminate_request scheduler::deliver (
        const std::string& r,
        transport::response_handler h)
{
    auto packed_request = std::make_shared<enqueued_request>(std::make_pair(r, h));
    boost::unique_lock<boost::mutex> serialize(mutex_);
    if (not shutting_down_) {
        auto queue_position = pending_requests_.insert(pending_requests_.end(), packed_request);
        if (not active_request_)
            run_next_request();
        
        typedef scheduler::option_to_terminate_request option;
        auto request_terminator = std::make_shared<option>(*this, packed_request, queue_position);
        return std::bind(&option::exercise, request_terminator, _1);
    } else {
        throw std::system_error(std::make_error_code(std::errc::network_down),
                "This single serial socket transport has been halted, and can no longer be used.");
    }
}


void scheduler::on_read (
        std::shared_ptr<scheduler::enqueued_request> intended_recipient,
        const boost::system::error_code& error,
        size_t n_read)
{
    boost::unique_lock<boost::mutex> serialize(mutex_);
    
    if (active_request_ == intended_recipient) {
        if (not error) {
            read_buffer_.commit(n_read);
            std::istream byte_stream(&read_buffer_);
            std::string received_bytes;
            received_bytes.resize(n_read);
            byte_stream.read(&received_bytes[0], n_read);
            
            // We schedule the new read in advance, because we want to be able to cancel a socket
            // operation to trigger request closure.
            auto this_ongoing_request = intended_recipient;
            auto on_read = std::bind(&scheduler::on_read, this, this_ongoing_request, _1, _2);
            socket_->async_read_some(read_buffer_, on_read);
            
            // The handler must be allowed to enqueue new requests recursively. Hence
            // the lack of serialization here.
            auto handler = active_request_->second;
            serialize.unlock();
#if _MSC_VER >= 1600
            auto error_code = std::make_error_code(static_cast<std::errc::errc>(error.value()));
#else
            auto error_code = std::make_error_code(static_cast<std::errc>(error.value()));
#endif
            handler(error_code, n_read, received_bytes);
        } else {
            handle_socket_error(error, std::move(serialize));
        }
    } else {
        // No closing of the connection: we moved on to the next request.
    }
}


void scheduler::on_write (
        std::shared_ptr<scheduler::enqueued_request> intended_recipient,
        const boost::system::error_code& error,
        size_t)
{
    boost::unique_lock<boost::mutex> serialize(mutex_);
    
    if (intended_recipient == active_request_) {
        if (not error) {
            auto on_read = std::bind(&scheduler::on_read, this, active_request_, _1, _2);
            socket_->async_read_some(read_buffer_, on_read);
        } else {
            handle_socket_error(error, std::move(serialize));
        }
    }
}


void scheduler::connect_socket ()
{
    resolver::iterator endpoint_iterator = resolver_->resolve(target_);
    resolver::iterator end;

    boost::system::error_code error = boost::asio::error::host_not_found;
    while (error && endpoint_iterator != end)
    {
        socket_->close();
        socket_->connect(*endpoint_iterator++, error);
    }

    if (error)
        throw boost::system::system_error(error);
}


void scheduler::handle_socket_error (const boost::system::error_code& error, boost::unique_lock<boost::mutex> serialized)
{
    assert(serialized);
    if (error == boost::asio::error::operation_aborted) {
        // For this error, we operate under the assumption that the request terminated with a
        // dirty connection. We need to completely kill and reconnect the socket, to avoid late
        // replies.
        //
        active_request_.reset();
        socket_->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        socket_->close();
        connect_socket();

        if (not shutting_down_)
            run_next_request();
    } else {
        // An actual error occurred, and we need to inform the application layer.
        //
        auto handler = active_request_->second;
#if _MSC_VER >= 1600
        auto error_code = std::make_error_code(static_cast<std::errc::errc>(error.value()));
#else
        auto error_code = std::make_error_code(static_cast<std::errc>(error.value()));
#endif

        // The handler may (actually: should) decide to terminate the request -- it must succeed.
        serialized.unlock();
        handler(error_code, 0, "");
    }
}


void scheduler::run_next_request ()
{
    if (not pending_requests_.empty()) {
        active_request_ = pending_requests_.front();
        pending_requests_.pop_front();
        auto on_write = std::bind(&scheduler::on_write, this, active_request_, _1, _2);
        asio::async_write(*socket_, asio::buffer(active_request_->first), on_write);
    } else {
        active_request_.reset();
    }
}


void scheduler::option_to_terminate_request::dequeue_thread_safely ()
{
    // TODO: What if this request just became active? Is that possible?
    boost::unique_lock<boost::mutex> serialize(pool_.mutex_);
    pool_.pending_requests_.erase(queue_position_);
    pool_.request_dequeued_.notify_one();
}


void scheduler::option_to_terminate_request::exercise (bool connection_is_dirty)
{
    boost::unique_lock<boost::mutex> serialize(this->mutex_);
    
    if (not exercised_) {
        if (pool_.active_request_->first == this_request_->first) {
            if (not connection_is_dirty) {
                boost::unique_lock<boost::mutex> serialize(pool_.mutex_);
                if (not pool_.shutting_down_)
                    pool_.run_next_request();
            }
            pool_.socket_->cancel();
        } else {
            auto dequeue = std::bind(&option_to_terminate_request::dequeue_thread_safely, shared_from_this());
            pool_.ios_.post(dequeue);
        }
        exercised_ = true;
    }
}

//=============================================================================
        }   // namespace single_serial_socket
    }   // namespace transport
}   // namespace riak
//=============================================================================
