#include <riak/message.hxx>
#include <sstream>
#include <system_error>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

//=============================================================================
namespace riak {
    namespace message {
        namespace {
//=============================================================================

/*!
 * Determines the incidence of a complete Riak message of the form
 *
 *     | Rest-of-Message Length (32 bits) | Message Code (8 bits) | Message Body |.
 *
 * \param begin must dereference to the first byte of an incoming sequence.
 * \param end must "point" to the byte immediately past the end of the received sequence.
 * \return An iterator which is equal to begin unless a complete response is available for
 *     consumption. In that case, it points to the character past the complete response.
 */
template <class iterator>
static
iterator next_partial_response (const iterator begin, const iterator end)
{
    assert(end >= begin);
    bool request_length_available = ((end - begin) >= static_cast<int64_t>(sizeof(uint32_t)));
    
    if (request_length_available) {
        uint32_t encoded_length = *reinterpret_cast<const uint32_t*>(&*begin);
        uint32_t data_length = ntohl(encoded_length);

        bool complete_response_available = (end - begin - sizeof(encoded_length) >= data_length);
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


bool handle_streaming_requests (
        std::shared_ptr<std::vector<unsigned char>> buffer,
        handler consume_complete_message,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& input)
{
    if (not error) {
        buffer->insert(buffer->end(), input.begin(), input.end());
        auto next_response = next_partial_response(buffer->begin(), buffer->end());
        bool matched_whole_request = (next_response != buffer->begin());
        
        if (matched_whole_request) {
            std::string one_request(buffer->begin(), next_response);
            buffer->erase(buffer->begin(), next_response);
            return consume_complete_message(error, one_request.size(), one_request);
        } else {
            // Wait for more data!
            return false;
        }
    } else {
        return consume_complete_message(error, 0, "");
    }
}

//=============================================================================
        }   // namespace (anonymous)
//=============================================================================

buffering_handler make_buffering_handler (handler& h)
{
    using namespace std::placeholders;
    
    auto buffer = std::make_shared<std::vector<unsigned char>>();
    return std::bind(&handle_streaming_requests, buffer, h, _1, _2, _3);
}

//=============================================================================
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


bool extract_code_if_valid (code& c, std::size_t bytes_available, const std::string& input)
{
    uint32_t encoded_length;
    if (bytes_available >= sizeof(encoded_length)) {
        std::istringstream input_data(input);
        input_data.read(reinterpret_cast<char*>(&encoded_length), sizeof(encoded_length));
        uint32_t data_length = ntohl(encoded_length);
        bytes_available -= sizeof(encoded_length);

        if (data_length == bytes_available) {
            uint8_t code_value;
            input_data.read(reinterpret_cast<char*>(&code_value), sizeof(code_value));
            c = code_value;
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}


template <typename PbMessageBody>
bool retrieve_body_with_code (
        code expected_code,
        PbMessageBody& body,
        std::size_t bytes_available,
        const std::string& input)
{
    code received_code;
    bool format_valid = extract_code_if_valid(received_code, bytes_available, input);
    if (format_valid and received_code == expected_code) {
        size_t size_of_header = 5;  // 32-bit length, 8-bit code
        std::string payload = input.substr(size_of_header);
        return body.ParseFromString(payload);
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
bool retrieve (pbtype& result, std::size_t n, const std::string& input) { \
    return retrieve_body_with_code(code::codename, result, n, input);    \
}

DECODE(RpbGetReq,  GetRequest );
DECODE(RpbGetResp, GetResponse);
DECODE(RpbPutReq,  PutRequest );
DECODE(RpbPutResp, PutResponse);

#undef ENCODE


bool verify_code(const code& expected_code, std::size_t bytes_received, const std::string& input) {
    code received_code;
    bool extraction_successful = extract_code_if_valid(received_code, bytes_received, input);
    return extraction_successful and (received_code == expected_code);
}


std::string wire_package::to_string () const
{
    uint32_t message_length = sizeof(static_cast<uint8_t>(message_code_)) + body_.size();
    uint32_t encoded_length = htonl(message_length);
    
    std::string full_message;
    full_message.append(reinterpret_cast<char*>(&encoded_length), sizeof(encoded_length));
    full_message += message_code_;
    full_message += body_;
    return full_message;
}

//=============================================================================
    }   // namespace message
}   // namespace riak
//=============================================================================
