#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>
#include <riak/request.hxx>
#include <riak/transports/single-serial-socket.hxx>
#include <system_error>

namespace asio = boost::asio;
using std::placeholders::_1;
using std::placeholders::_2;

//=============================================================================
namespace riak {
//=============================================================================

single_serial_socket::single_serial_socket (
        const std::string& node_address,
        uint16_t port,
        boost::asio::io_service& ios)
  : target_(node_address, boost::lexical_cast<std::string>(port))
  , ios_(ios)
  , socket_(ios)
{
    connect_socket();
}


single_serial_socket::~single_serial_socket ()
{
    boost::unique_lock<boost::mutex> serialize(mutex_);
    
    // Prevent new requests from entering, so we don't race over the pending_requests_ list.
    bool shutting_down_ = true;
    
    // Report the shutdown to all clients.
    for (auto entry = pending_requests_.begin(); entry != pending_requests_.end(); ++entry) {
        auto pending_request = *entry;
        auto response_handler = pending_request->second;
        response_handler(std::make_error_code(std::errc::network_reset), 0, "");
    }
    
    while (not pending_requests_.empty())
        request_dequeued_.wait(serialize);
    
    // Requests may have been made active while we were cleaning, so this has to be last!
    socket_.cancel();
    while (active_request_)
        active_request_finished_.wait(serialize);
    
    using boost::asio::ip::tcp;
    socket_.shutdown(tcp::socket::shutdown_both);
    socket_.close();
}


std::shared_ptr<transport::option_to_terminate_request> single_serial_socket::deliver (
        std::shared_ptr<const request> r,
        response_handler h)
{
    boost::unique_lock<boost::mutex> serialize(mutex_);
    if (not shutting_down_) {
        auto request_enqueued = std::make_shared<enqueued_request>(std::make_pair(r, h));
        auto queue_position = pending_requests_.insert(pending_requests_.end(), request_enqueued);
        if (not active_request_)
            run_next_request();
        
        typedef single_serial_socket::option_to_terminate_request option;
        return std::shared_ptr<option>(new option(*this, std::move(request_enqueued), queue_position));
    } else {
        throw std::system_error(std::make_error_code(std::errc::network_down),
                "This single serial socket transport has been halted, and can no longer be used.");
    }
}


void single_serial_socket::on_read (
        std::shared_ptr<single_serial_socket::enqueued_request> intended_recipient,
        const boost::system::error_code& error,
        size_t n_read)
{
    boost::unique_lock<boost::mutex> serialize(mutex_);
    
    if (active_request_ == intended_recipient) {
        if (not error) {
            read_buffer_.commit(n_read);
            std::istream byte_stream(&read_buffer_);
            std::ostringstream reader;
            reader << byte_stream.rdbuf();
            read_buffer_.consume(n_read);
            
            // We schedule the new read in advance, because we want to be able to cancel a socket
            // operation to trigger request closure.
            auto this_ongoing_request = intended_recipient;
            auto on_read = std::bind(&single_serial_socket::on_read, this, this_ongoing_request, _1, _2);
            socket_.async_read_some(read_buffer_.prepare(1024), on_read);
            
            // The handler must be allowed to enqueue new requests recursively. Hence
            // the lack of serialization here.
            auto handler = active_request_->second;
            serialize.unlock();
            auto error_code = std::make_error_code(static_cast<std::errc>(error.value()));
            handler(error_code, n_read, reader.str());
        } else {
            handle_socket_error(error);
        }
    }
}


void single_serial_socket::on_write (
        std::shared_ptr<single_serial_socket::enqueued_request> intended_recipient,
        const boost::system::error_code& error,
        size_t)
{
    boost::unique_lock<boost::mutex> serialize(mutex_);
    
    if (intended_recipient == active_request_) {
        if (not error) {
            auto on_read = std::bind(&single_serial_socket::on_read, this, active_request_, _1, _2);
            socket_.async_read_some(read_buffer_.prepare(1024), on_read);
        } else {
            handle_socket_error(error);
        }
    }
}


void single_serial_socket::connect_socket ()
{
    using boost::asio::ip::tcp;
    tcp::resolver resolver(ios_);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(target_);
    boost::asio::connect(socket_, endpoint_iterator);
}


void single_serial_socket::handle_socket_error (const boost::system::error_code& error)
{
    if (error == boost::asio::error::operation_aborted) {
        // For this error, the request timed out at the Riak::Store level.
        // Once an option to terminate has been exercised, we cannot call the handler any more.
        //
        active_request_.reset();

        // We need to completely kill and reconnect the socket, to avoid late replies.
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        socket_.close();
        connect_socket();

        if (not shutting_down_ and not pending_requests_.empty())
            run_next_request();
    } else {
        // An actual error occurred, and we need to give the riak::store a chance to release us.
        //
        auto handler = active_request_->second;
        auto error_code = std::make_error_code(static_cast<std::errc>(error.value()));
        handler(error_code, 0, "");
    }
}


void single_serial_socket::run_next_request ()
{
    active_request_ = pending_requests_.front();
    pending_requests_.pop_front();
    auto on_write = std::bind(&single_serial_socket::on_write, this, active_request_, _1, _2);
    asio::async_write(socket_, asio::buffer(active_request_->first->payload()), on_write);
}


void single_serial_socket::option_to_terminate_request::dequeue_thread_safely ()
{
    // TODO: What if this request just became active? Is that possible?
    boost::unique_lock<boost::mutex> serialize(pool_.mutex_);
    pool_.pending_requests_.erase(queue_position_);
    pool_.request_dequeued_.notify_one();
}


void single_serial_socket::option_to_terminate_request::exercise ()
{
    boost::unique_lock<boost::mutex> serialize(this->mutex_);
    
    if (not exercised_) {
        if (pool_.active_request_->first == this_request_->first) {
            pool_.socket_.cancel();
        } else {
            auto dequeue = std::bind(&option_to_terminate_request::dequeue_thread_safely, shared_from_this());
            pool_.ios_.post(dequeue);
        }
        exercised_ = true;
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
