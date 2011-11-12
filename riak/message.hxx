#pragma once
#include <cstddef>
#include <functional>
#include <memory>
#include <riak/riakclient.pb.h>
#include <string>

//=============================================================================
namespace riak {
    namespace message {
//=============================================================================

/*!
 * Such a handler should return according with whether the request has been completely
 * satisfied (i.e. no additional responses are expected). A return of true indicates
 * the end of the request, and should free transport resources. If such a handler is
 * invoked _without_ error, it is guaranteed that the given string constitutes a
 * complete Riak message of the form:
 *
 *     | Rest-of-Message Length (32 bits) | Message Code (8 bits) | Message Body |
 *
 * In this case, the given size will equal (rest_of_message_length + sizeof(uint32_t)).
 *
 * Beware: It does not guarantee whether that response's payload is sensible. The handler
 * is responsible for such validation as: is the response of the message type expected?
 * Is the body correctly encoded?
 */
typedef std::function<bool(std::error_code, std::size_t, const std::string&)> handler;

/*!
 * Exactly as handler, but prepared to accept any data input, including partial responses.
 */
typedef handler buffering_handler;

/*!
 * Builds a buffering handler which can accept streaming data from a network and collect
 * whole requests from that data.
 *
 * \param h will be called only when the returned handler has received a full Riak
 *     message as per the definition of handler. It will also be called in case an
 *     error is given from the caller.
 */
buffering_handler make_buffering_handler (handler& h);

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
bool retrieve (PbMessageBody& body, std::size_t bytes_received, const std::string& data);

template <> bool retrieve (const RpbGetResp&, std::size_t, const std::string&);
template <> bool retrieve (const RpbPutResp&, std::size_t, const std::string&);

bool verify_code (const code& c, std::size_t, const std::string&);
    
//=============================================================================
    }   // namespace message
}   // namespace riak
//=============================================================================
