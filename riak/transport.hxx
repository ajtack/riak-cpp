#pragma once
#include <memory>
#include <riak/request.hxx>

//=============================================================================
namespace riak {
//=============================================================================

class transport
{
  public:
    class option_to_terminate_request;
    
    typedef std::function<void(std::error_code, std::size_t, const std::string&)> response_handler;
      
    virtual ~transport ()
    {   }
    
    /*!
     * Dispatches the given request at the next available opportunity. This function
     * may optionally return immediately, constituting asynchronous behavior.
     *
     * \param r may optionally be maintained by the connection pool beyond this method call.
     * \param h must always be called to indicate either failure or success, including upon
     *     destruction of the connection pool prior to resolution of a request. Multiple calls
     *     are permissible, and calls with empty payloads will affect timeouts.
     */
    virtual std::shared_ptr<option_to_terminate_request> deliver (
            std::shared_ptr<const request> r,
            response_handler h) = 0;
};


class transport::option_to_terminate_request
{
  public:
    virtual ~option_to_terminate_request ()
    {   }
    
    /*!
     * Indicates to the connection pool that the associated request has ended, whether successfully
     * or otherwise. The connection pool may cancel any ongoing requests by the mechanism it
     * wishes. Following the exercise of a cancel option, the connection pool guarantees it
     * will make no further callbacks related to the associated request.
     *
     * This signal must be idempotent.
     */
    virtual void exercise () = 0;
};

//=============================================================================
}   // namespace riak
//=============================================================================
