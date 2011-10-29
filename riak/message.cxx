#include <boost/asio/streambuf.hpp>
#include <iostream>
#include <riak/message.hxx>

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


bool extract_code_if_valid (code& c, std::size_t bytes_available, boost::asio::streambuf& input)
{
    uint32_t encoded_length;
    assert (bytes_available >= sizeof(encoded_length));
    input.commit(sizeof(encoded_length));
    bytes_available -= sizeof(encoded_length);
    std::istream input_data(&input);
    input_data.read(reinterpret_cast<char*>(&encoded_length), sizeof(encoded_length));
    uint32_t data_length = ntohl(encoded_length);
    
    if (data_length == bytes_available) {
        uint8_t code_value;
        input.commit(data_length);
        input_data.read(reinterpret_cast<char*>(&code_value), sizeof(code_value));
        c = code_value;
        return true;
    } else {
        return false;
    }
}


template <typename PbMessageBody>
bool retrieve_body_with_code (code expected_code, PbMessageBody& body, std::size_t bytes_available,
        boost::asio::streambuf& input)
{
    code received_code;
    bool format_valid = extract_code_if_valid(received_code, bytes_available, input);
    if (format_valid and received_code == expected_code) {
        std::istream input_stream(&input);
        body.ParseFromIstream(&input_stream);
        return true;
    } else {
        return false;
    }
}
    

//=============================================================================
        }   // namespace (anonymous)
//=============================================================================

const code code::GetRequest(9);
const code code::GetResponse(10);
const code code::PutRequest(11);
const code code::PutResponse(12);
const code code::DeleteRequest(13);
const code code::DeleteResponse(14);

#define ENCODE(pbtype, codename)                 \
template <>                                      \
wire_package encode (const pbtype& b) {          \
    return package_with_code(code::codename, b); \
}

ENCODE(RpbGetReq, GetRequest);
ENCODE(RpbPutReq, PutRequest);
ENCODE(RpbDelReq, DeleteRequest);

#undef ENCODE


#define DECODE(pbtype, codename)                             \
template <>                                                  \
bool retrieve (pbtype& result, std::size_t n, boost::asio::streambuf& input) { \
    return retrieve_body_with_code(code::codename, result, n, input);    \
}

DECODE(RpbGetResp, GetResponse);
DECODE(RpbPutResp, PutResponse);

#undef ENCODE


bool verify_code(code c, std::size_t bytes_received, boost::asio::streambuf& input) {
    return extract_code_if_valid(c, bytes_received, input);
}


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