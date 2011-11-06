#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <list>
#include <memory>
#include <riak/transport.hxx>

//=============================================================================
namespace riak {
//=============================================================================

class single_serial_socket
      : public transport
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
    
    /*!
     * Dispatches the given request at the next available opportunity. This function
     * may optionally return immediately, constituting asynchronous behavior.
     *
     * \param r may optionally be maintained by the connection pool beyond this method call.
     * \param h must always be called to indicate either failure or success, including upon
     *     destruction of the connection pool prior to resolution of a request. Multiple calls
     *     are permissible, and calls with empty payloads will affect timeouts.
     */
    virtual std::shared_ptr<transport::option_to_terminate_request> deliver (
            std::shared_ptr<const request> r,
            response_handler h);
    
  private:
    class option_to_terminate_request;
    friend class option_to_terminate_request;
    typedef std::pair<std::shared_ptr<const request>, response_handler> enqueued_request;
    
    boost::asio::io_service& ios_;
    boost::asio::ip::tcp::resolver::query target_;
    
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
};


class single_serial_socket::option_to_terminate_request
      : public transport::option_to_terminate_request
      , public std::enable_shared_from_this<option_to_terminate_request>
{
  public:
    virtual ~option_to_terminate_request () {
        exercise();
    }
    
    /*!
     * Indicates to the connection pool that the associated request has ended, whether successfully
     * or otherwise. The connection pool may cancel any ongoing requests by the mechanism it
     * wishes. Following the exercise of a cancel option, the connection pool guarantees it
     * will make no further callbacks related to the associated request.
     *
     * This signal must be idempotent.
     */
    virtual void exercise ();
    
  protected:
    friend class single_serial_socket;
    option_to_terminate_request (
            single_serial_socket& p,
            std::shared_ptr<single_serial_socket::enqueued_request>&& r,
            const single_serial_socket::request_queue::iterator pos)
      : pool_(p)
      , this_request_(std::move(r))
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
}   // namespace riak
//=============================================================================
