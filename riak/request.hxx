#pragma once
#include <string>
#include <system_error>

//=============================================================================
namespace riak {
//=============================================================================

class request
{
public:
    typedef std::function<void(std::error_code, std::size_t, const std::string&)> response_handler;
    
    virtual ~request ()
    {   }
    
    /*! Yields the character sequence that shall be transmitted unmodified to the server. */
    virtual const std::string& payload () const = 0;
    
    /*!
     * Intended to delay reception until the response constitutes a complete Riak command of the form
     * | Rest-of-Message Length (32 bits) | Message Code (8 bits) | Message Body |. Implementations
     * of transport should leverage this when accepting streams in response to a request.
     *
     * \param begin must dereference to the byte of an incoming sequence.
     * \param end must "point" to the byte immediately past the end of the received sequence.
     * \return A pair of <beginning, request_matched> where request_matched is true if and only
     *     if a well-formed request was identified in the given sequence.
     */
    template <class iterator>
    static
    std::pair<iterator, bool> is_well_formed (const iterator begin, const iterator end);
};


template <class iterator>
std::pair<iterator, bool> request::is_well_formed (const iterator begin, const iterator end)
{
    assert(end >= begin);
    if (end - begin >= sizeof(uint32_t) + sizeof(char)) {
        uint32_t encoded_length = *reinterpret_cast<const uint32_t*>(&*begin);
        uint32_t data_length = ntohl(encoded_length);

        if (end - (begin + sizeof(encoded_length)) >= data_length) {
            // Request is well-formed.
            iterator new_beginning = begin + sizeof(encoded_length) + data_length;
            return std::make_pair(new_beginning, true);
        } else {
            return std::make_pair(begin, false);
        }
    } else {
        return std::make_pair(begin, false);
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
