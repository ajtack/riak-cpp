#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/mutex.hpp>
#include <chrono>
#include <memory>

namespace boost {
    namespace asio { class io_service; }
}

//=============================================================================
namespace riak {
//=============================================================================

class request_with_timeout
      : public std::enable_shared_from_this<request_with_timeout>
{
  public:
    typedef std::function<void(const boost::system::error_code&, std::size_t, boost::asio::streambuf&)> response_handler;
      
    /*!
     * Packages a task, but does not send it or begin any timeout counting.
     * \param timeout is in milliseconds.
     */
    request_with_timeout (
            const std::string& data,
            std::chrono::milliseconds timeout,
            boost::asio::ip::tcp::socket& s,
            response_handler& h,
            boost::asio::io_service& ios);
    
    /*!
     * Sends the request and begins listening for a response asynchronously. Timeout countdown starts now.
     * \pre There must exist a shared pointer to this request at the time of dispatch. It may be
     *     reset as soon as the request is dispatched.
     */
    void dispatch ();

  private:
    void on_response (const boost::system::error_code&, std::size_t);
    void on_timeout (const boost::system::error_code&);
    void on_write (const boost::system::error_code&, std::size_t);
      
    mutable boost::mutex mutex_;
    boost::asio::ip::tcp::socket& socket_;
    std::chrono::milliseconds timeout_length_;
    boost::asio::deadline_timer timeout_;
    response_handler response_callback_;
    const std::string request_data_;
    boost::asio::streambuf response_data_;
    
    bool succeeded_;
    bool timed_out_;
};

//=============================================================================
}   // namespace riak
//=============================================================================
