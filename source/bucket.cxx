/*!
 * \file
 * Implementation as per header. Currently excludes real functionality (play with hash table).
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
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
        const std::string& data)
{
    if (not error) {
        p->set_value();
    } else {
        p->set_exception(boost::copy_exception(std::runtime_error("Darn Error")));
    }
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
    std::string encoded_request;
    request.SerializeToString(&encoded_request);
    
    std::shared_ptr<boost::promise<void>> promise(new boost::promise<void>());
    
    using std::placeholders::_1;
    using std::placeholders::_2;
    store::response_handler callback = std::bind(&delete_handler_for_promise, promise, _1, _2);
    store_.transmit_request(encoded_request, callback);
    
    return promise->get_future();
}

//=============================================================================
}   // namespace riak
//=============================================================================
