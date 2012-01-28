#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/lexical_cast.hpp>
#include <riak/transports/single-serial-socket.hxx>
#include <system_error>

namespace asio = boost::asio;

//=============================================================================
namespace riak {
    namespace {
//=============================================================================

class single_serial_socket
      : public std::enable_shared_from_this<single_serial_socket>
{
  public:
    /*!
     * \param node_address should provide the location of a Riak node at which requests may be made.
     *     Will be resolved via DNS.
     * \param port identifies the port at which the service is accessed.
     * \param ios must survive through the destruction of this transport.
     * \post A connection to node_address is made eagerly at the given location. The transport is ready
     *     to deliver requests.
     */
    single_serial_socket (
            const std::string& node_address,
            uint16_t port,
            boost::asio::io_service& ios);
    
    virtual ~single_serial_socket ();

    virtual transport::option_to_terminate_request deliver (
            const std::string& r,
            transport::response_handler h);
    
  private:
    class option_to_terminate_request;
    friend class option_to_terminate_request;
    typedef std::pair<const std::string, transport::response_handler> enqueued_request;
    
    boost::asio::ip::tcp::resolver::query target_;
    boost::asio::io_service& ios_;
    
    mutable boost::mutex mutex_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::streambuf read_buffer_;
    std::shared_ptr<enqueued_request> active_request_;
    boost::condition_variable active_request_finished_;
    
    typedef std::list<std::shared_ptr<enqueued_request>> request_queue;
    request_queue pending_requests_;
    boost::condition_variable request_dequeued_;
    
    bool shutting_down_;
    
    void on_read (std::shared_ptr<enqueued_request>, const boost::system::error_code&, size_t);
    void on_write (std::shared_ptr<enqueued_request>, const boost::system::error_code&, size_t);
    void run_next_request ();
    void handle_socket_error (const boost::system::error_code&);
    void connect_socket ();
    transport::option_to_terminate_request enqueue (enqueued_request&, const request_queue::iterator&);
};

class single_serial_socket::option_to_terminate_request
      : public std::enable_shared_from_this<single_serial_socket::option_to_terminate_request>
{
  public:
    virtual ~option_to_terminate_request () {
        exercise();
    }
    
    virtual void exercise ();
    
    option_to_terminate_request (
            single_serial_socket& p,
            std::shared_ptr<single_serial_socket::enqueued_request>& r,
            const single_serial_socket::request_queue::iterator pos)
      : pool_(p)
      , this_request_(r)
      , queue_position_(pos)
      , exercised_(false)
    {   }
    
  private:
    // The option cannot be serialized with the single_serial_socket's mutex, because a
    // response handler will invoke this with that locked. Instead, we rely on
    // pool_.socket working asynchronously.
    boost::mutex mutex_;
    single_serial_socket& pool_;
    std::shared_ptr<single_serial_socket::enqueued_request> this_request_;
    const single_serial_socket::request_queue::iterator queue_position_;
    bool exercised_;
    
    void dequeue_thread_safely ();
};

//=============================================================================
    }   // namespace (anonymous)
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;

transport::delivery_provider make_single_socket_transport (
        const std::string& address,
        uint16_t port,
        boost::asio::io_service& ios)
{
    auto transport = std::make_shared<single_serial_socket>(address, port, ios);
    return std::bind(&single_serial_socket::deliver, transport, _1, _2);
}


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


transport::option_to_terminate_request single_serial_socket::deliver (
        const std::string& r,
        transport::response_handler h)
{
    auto packed_request = std::make_shared<enqueued_request>(std::make_pair(r, h));
    boost::unique_lock<boost::mutex> serialize(mutex_);
    if (not shutting_down_) {
        auto queue_position = pending_requests_.insert(pending_requests_.end(), packed_request);
        if (not active_request_)
            run_next_request();
        
        typedef single_serial_socket::option_to_terminate_request option;
        auto request_terminator = std::make_shared<option>(*this, packed_request, queue_position);
        return std::bind(&option::exercise, request_terminator);
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
#if _MSC_VER == 1600
            auto error_code = std::make_error_code(static_cast<std::errc::errc>(error.value()));
#else
            auto error_code = std::make_error_code(static_cast<std::errc>(error.value()));
#endif
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
    socket_.connect(*endpoint_iterator);
}


void single_serial_socket::handle_socket_error (const boost::system::error_code& error)
{
    if (error == boost::asio::error::operation_aborted) {
        // For this error, the request timed out at the riak::client level.
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
#if _MSC_VER == 1600
        auto error_code = std::make_error_code(static_cast<std::errc::errc>(error.value()));
#else
        auto error_code = std::make_error_code(static_cast<std::errc>(error.value()));
#endif
        handler(error_code, 0, "");
    }
}


void single_serial_socket::run_next_request ()
{
    active_request_ = pending_requests_.front();
    pending_requests_.pop_front();
    auto on_write = std::bind(&single_serial_socket::on_write, this, active_request_, _1, _2);
    asio::async_write(socket_, asio::buffer(active_request_->first), on_write);
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
