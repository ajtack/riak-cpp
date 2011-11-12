#include <boost/asio/deadline_timer.hpp>
#include <boost/thread/mutex.hpp>
#include <chrono>
#include <memory>
#include <riak/message.hxx>
#include <riak/transport.hxx>
#include <riak/request.hxx>
#include <system_error>

namespace boost {
    namespace asio { class io_service; }
}

//=============================================================================
namespace riak {
//=============================================================================

class request_with_timeout
      : public std::enable_shared_from_this<request_with_timeout>
      , public riak::request
{
  public:
    /*!
     * Packages a task, but does not send it or begin timeout calculations.
     * \param timeout is in milliseconds. It determines the maximum length of any radio silence
     *     from the server.
     */
    request_with_timeout (
            const std::string& data,
            std::chrono::milliseconds timeout,
            message::buffering_handler& h,
            boost::asio::io_service& ios);
    
    /*!
     * Sends the request and begins listening for a response asynchronously. Timeout countdown starts now.
     * \pre There must exist a shared pointer to this request at the time of dispatch. It may be
     *     reset as soon as the request is dispatched.
     */
    void dispatch_via (transport& p);
    
    virtual const std::string& payload () const { return request_data_; }

  private:
    void on_response (std::error_code, std::size_t, const std::string&);
    void on_timeout (const boost::system::error_code&);
      
    mutable boost::mutex mutex_;
    std::chrono::milliseconds timeout_length_;
    boost::asio::deadline_timer timeout_;
    message::buffering_handler response_callback_;
    std::shared_ptr<transport::option_to_terminate_request> option_to_terminate_request_;
    const std::string request_data_;
    
    bool succeeded_;
    bool timed_out_;
};

//=============================================================================
}   // namespace riak
//=============================================================================
