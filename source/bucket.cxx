/*!
 * \file
 * Implementation as per header. Currently excludes real functionality (play with hash table).
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#include <boost/asio/streambuf.hpp>
#include <boost/lexical_cast.hpp>
#include <bucket.hxx>
#include <riakclient.pb.h>
#include <store.hxx>
#include <string>

//=============================================================================
namespace riak {
//=============================================================================

object bucket::operator[] (const ::riak::key& k)
{
    return object(store_, *this, k);
}

//=============================================================================
    namespace {
//=============================================================================

void delete_handler_for_promise (
        std::shared_ptr<boost::promise<void>>& p,
        const boost::system::error_code& error,
        size_t bytes_received,
        std::shared_ptr<boost::asio::streambuf> data)
{
    if (not error) {
        p->set_value();
    } else {
        p->set_exception(boost::copy_exception(std::runtime_error("Darn Error")));
    }
}


std::string packaged_delete_request (RpbDelReq& r)
{
    using std::string;
    string encoded_body;
    r.SerializeToString(&encoded_body);
    
    char message_code = 13;
    uint32_t message_length = htonl(sizeof(message_code) + encoded_body.size());
    
    std::string full_message;
    full_message.append(reinterpret_cast<char*>(&message_length), sizeof(message_length));
    full_message += message_code;
    full_message += encoded_body;
    return full_message;
}

//=============================================================================
    }   // namespace (anonymous)
//=============================================================================

boost::unique_future<void> bucket::unmap (const ::riak::key& k)
{
    RpbDelReq request;
    request.set_bucket(this->key());
    request.set_key(k);
    request.set_r (default_access_parameters_.r);
    request.set_rw(default_access_parameters_.rw);
    request.set_w (default_access_parameters_.w);
    request.set_dw(default_access_parameters_.dw);
    request.set_pr(default_access_parameters_.pr);
    request.set_pw(default_access_parameters_.pw);
    
    std::shared_ptr<boost::promise<void>> promise(new boost::promise<void>());
    
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    std::shared_ptr<boost::asio::streambuf> buffer(new boost::asio::streambuf);
    store::response_handler callback = std::bind(&delete_handler_for_promise, promise, _1, _2, _3);
    store_.transmit_request(packaged_delete_request(request), buffer, callback);
    
    return promise->get_future();
}

//=============================================================================
}   // namespace riak
//=============================================================================
