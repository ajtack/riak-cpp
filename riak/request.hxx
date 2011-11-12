#pragma once
#include <string>

//=============================================================================
namespace riak {
//=============================================================================

class request
{
public:
    virtual ~request ()
    {   }
    
    /*! Yields the character sequence that shall be transmitted unmodified to the server. */
    virtual const std::string& payload () const = 0;
};

//=============================================================================
}   // namespace riak
//=============================================================================
