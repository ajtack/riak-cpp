#pragma once
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <riak/transport.hxx>
#include <list>
#include <string>

namespace riak {
  namespace transport {
    namespace single_serial_socket {
      class socket;
      class resolver;
    }
  }
}

//=============================================================================
namespace riak {
	namespace transport {
		namespace single_serial_socket {
//=============================================================================

class scheduler
      : public std::enable_shared_from_this<scheduler>
{
  public:
    /*!
     * \param node_address should provide the location of a Riak node at which requests may be made.
     *     Will be resolved via DNS.
     * \param port identifies the port at which the service is accessed.
     * \param ios must survive through the destruction of this transport.
     * \param s will be the physical socket used in this pool.
     * \param resolver will be used to resolve node_address and port upon every reconnection.
     * \post A connection to node_address is made eagerly at the given location. The transport is ready
     *     to deliver requests, or construction will have thrown.
     */
    scheduler (
            const std::string& node_address,
            uint16_t port,
            boost::asio::io_service& ios,
            std::unique_ptr<socket> s,
            const std::shared_ptr<resolver>& resolver);
    
    virtual ~scheduler ();

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
    std::unique_ptr<socket> socket_;
    std::shared_ptr<resolver> resolver_;
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
    void handle_socket_error (const boost::system::error_code&, boost::unique_lock<boost::mutex>);
    void connect_socket ();
    transport::option_to_terminate_request enqueue (enqueued_request&, const request_queue::iterator&);
};

class scheduler::option_to_terminate_request
      : public std::enable_shared_from_this<scheduler::option_to_terminate_request>
{
  public:
    virtual ~option_to_terminate_request () {
        exercise(false);
    }
    
    virtual void exercise (bool connection_is_dirty);
    
    option_to_terminate_request (
            scheduler& p,
            std::shared_ptr<scheduler::enqueued_request>& r,
            const scheduler::request_queue::iterator pos)
      : pool_(p)
      , this_request_(r)
      , queue_position_(pos)
      , exercised_(false)
    {   }
    
  private:
    // The option cannot be serialized with the scheduler's mutex, because a
    // response handler will invoke this with that locked. Instead, we rely on
    // pool_.socket working asynchronously.
    boost::mutex mutex_;
    scheduler& pool_;
    std::shared_ptr<scheduler::enqueued_request> this_request_;
    const scheduler::request_queue::iterator queue_position_;
    bool exercised_;
    
    void dequeue_thread_safely ();
};

//=============================================================================
		}   // namespace single_serial_socket
	}   // namespace transport
}   // namespace riak
//=============================================================================
