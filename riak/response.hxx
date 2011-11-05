#pragma once

//=============================================================================
namespace riak {
//=============================================================================

class response
{
public:
    /*!
     * Intended to delay reception until the response constitutes a complete Riak command of the form
     * | Rest-of-Message Length (32 bits) | Message Code (8 bits) | Message Body |. Implementations
     * of transport should leverage this when accepting streams in response to a response.
     *
     * \param begin must dereference to the first byte of an incoming sequence.
     * \param end must "point" to the byte immediately past the end of the received sequence.
     * \return An iterator which is equal to begin unless a complete response is available for
     *     consumption. In that case, it points to the character past the complete response.
     */
    template <class iterator>
    static
    iterator next_partial_response (const iterator begin, const iterator end);
};


template <class iterator>
iterator response::next_partial_response (const iterator begin, const iterator end)
{
    assert(end >= begin);
    bool request_length_available = ((end - begin) >= sizeof(uint32_t));
    
    if (request_length_available) {
        uint32_t encoded_length = *reinterpret_cast<const uint32_t*>(&*begin);
        uint32_t data_length = ntohl(encoded_length);

        bool complete_response_available = (end - (begin + sizeof(encoded_length)) >= data_length);
        if (complete_response_available) {
            iterator new_beginning = begin + sizeof(encoded_length) + data_length;
            return new_beginning;
        } else {
            return begin;
        }
    } else {
        return begin;
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
