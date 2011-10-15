#include <message.hxx>
#include <iostream>

//=============================================================================
namespace riak {
    namespace message {
        namespace {
//=============================================================================

template <typename PbMessageBody>
wire_package package_with_code (code message_code, const PbMessageBody& b)
{
    using std::string;
    string encoded_body;
    b.SerializeToString(&encoded_body);
    
    return wire_package(message_code, encoded_body);
}

//=============================================================================
        }   // namespace (anonymous)
//=============================================================================

const code code::DeleteRequest(13);


template <>
wire_package encode (const RpbDelReq& b) { return package_with_code(code::DeleteRequest, b); }


std::string wire_package::to_string () const
{
    uint32_t message_length = htonl(sizeof(static_cast<uint8_t>(message_code_)) + body_.size());
    
    std::string full_message;
    full_message.append(reinterpret_cast<char*>(&message_length), sizeof(message_length));
    full_message += message_code_;
    full_message += body_;
    return full_message;
}

//=============================================================================
    }   // namespace message
}   // namespace riak
//=============================================================================
