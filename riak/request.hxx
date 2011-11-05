#pragma once
#include <string>
#include <system_error>

//=============================================================================
namespace riak {
//=============================================================================

class request
{
public:
    /*!
     * Such a handler should return according with whether the request has been completely
     * satisfied. A return of true will end the request, freeing transport resources.
     */
    typedef std::function<bool(std::error_code, std::size_t, const std::string&)> response_handler;
    
    virtual ~request ()
    {   }
    
    /*! Yields the character sequence that shall be transmitted unmodified to the server. */
    virtual const std::string& payload () const = 0;
};

//=============================================================================
}   // namespace riak
//=============================================================================
