/*!
 * \file
 * Provides facilities for encoding of Riak protocol buffer messages into over-the-network
 * transmittable queries.
 *
 * \author Andres Jaan tack <ajtack@gmail.com>
 */
#pragma once
#include <boost/asio/streambuf.hpp>
#include <cstddef>
#include <riak/riakclient.pb.h>

//=============================================================================
namespace riak {
    namespace message {
//=============================================================================

/*! Specifies the integer code used to identify a message. These values are copy/pasted from riakclient.proto. */
struct code
{
    static const code GetRequest;
    static const code GetResponse;
    static const code PutRequest;
    static const code PutResponse;
    static const code DeleteRequest;
    static const code DeleteResponse;
    
    operator std::uint8_t () const { assert(valid_); return value_; }
    
    code ()
      : valid_(false)
    {   }
    
    code& operator= (uint8_t val) {
        value_ = val;
        valid_ = true;
    }

private:
    explicit code (uint8_t v)
      : value_(v),
        valid_(true)
    {   }
    
    uint8_t value_;
    bool valid_;
};

/*! Represents a valid Riak message packaged and correctly formatted for over-the-wire transmission. */
class wire_package
{
public:
    wire_package (code c, const std::string& message)
      : message_code_(c),
        body_(message)
    {   }

    std::uint8_t message_code () const { return message_code_; }

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

template <> wire_package encode (const RpbGetReq&);
template <> wire_package encode (const RpbPutReq&);
template <> wire_package encode (const RpbDelReq&);

/*!
 * Accepts a buffer and produces from it a Protocol Buffer structure which is verified to have arrived
 * with an acceptable wire encoding.
 * \return true iff decoding of the given protocol buffer was successful.
 */
template <typename PbMessageBody>
bool retrieve (PbMessageBody& body, std::size_t bytes_received, boost::asio::streambuf& data);

template <> bool retrieve (const RpbGetResp&, std::size_t, boost::asio::streambuf&);
template <> bool retrieve (const RpbPutResp&, std::size_t, boost::asio::streambuf&);

bool verify_code (code c, std::size_t, boost::asio::streambuf&);
    
//=============================================================================
    }   // namespace message
}   // namespace riak
//=============================================================================
