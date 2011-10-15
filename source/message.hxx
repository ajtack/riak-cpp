/*!
 * \file
 * Provides facilities for encoding of Riak protocol buffer messages into over-the-network
 * transmittable queries.
 *
 * \author Andres Jaan tack <ajtack@gmail.com>
 */
#pragma once
#include <cstddef>
#include <riakclient.pb.h>

//=============================================================================
namespace riak {
    namespace message {
//=============================================================================

/*! Specifies the integer code used to identify a message. These values are copy/pasted from riakclient.proto. */
struct code
{
    static const code DeleteRequest;
    operator std::uint8_t () const { return value_; }

private:
    explicit code (uint8_t v)
      : value_(v)
    {   }
    
    uint8_t value_;
};

/*! Represents a valid Riak message packaged and correctly formatted for over-the-wire transmission. */
class wire_package
{
public:
    wire_package (code c, const std::string& message)
      : message_code_(c),
        body_(message)
    {   }

    std::size_t message_code () const { return message_code_; }

    /*! Produces the over-the-wire transmittable message message. */
    std::string to_string () const;

private:
    const code message_code_;
    const std::string body_;
};

/*!
 * Deduces the type of request to send from the parameter type. Produces a (lazily-) encoded message
 * specifying a message for over-the-wire transmission.
 * \param b must be fully specified as the desired encoding should reflect.
 */
template <typename PbMessageBody>
wire_package encode (const PbMessageBody& b);

template <> wire_package encode (const RpbDelReq&);
    
//=============================================================================
    }   // namespace message
}   // namespace riak
//=============================================================================
