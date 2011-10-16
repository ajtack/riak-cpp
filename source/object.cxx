/*!
 * \file
 * Implements asynchronous communication with a Riak server over objects.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#include <boost/asio/streambuf.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/mutex.hpp>
#include <bucket.hxx>
#include <message.hxx>
#include <object.hxx>
#include <riakclient.pb.h>
#include <store.hxx>

//=============================================================================
namespace riak {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


boost::shared_future<boost::optional<object::siblings>> object::fetch () const
        // const object_access_parameters& p = store_.object_access_defaults() ) const
{
    RpbGetReq request;
    request.set_bucket(this->bucket_);
    request.set_key(this->key());
    request.set_r (default_access_parameters_.r);
    request.set_pr(default_access_parameters_.pr);
    request.set_basic_quorum(default_access_parameters_.basic_quorum);
    request.set_notfound_ok(default_access_parameters_.notfound_ok);
    if (cached_vector_clock_) request.set_if_modified(*cached_vector_clock_);
    request.set_head(false);
    request.set_deletedvclock(true);
    auto query = message::encode(request);
    
    typedef boost::optional<object::siblings> optval;
    std::shared_ptr<boost::promise<optval>> promise(new boost::promise<optval>());
    store::response_handler callback = std::bind(&object::on_fetch_response, shared_from_this(), promise, _1, _2, _3);
    store_.transmit_request(query.to_string(), callback, default_request_failure_parameters_.response_timeout);
    return promise->get_future();
}


void object::on_fetch_response (
        std::shared_ptr<boost::promise<boost::optional<object::siblings>>>& p,
        const boost::system::error_code& error,
        std::size_t bytes_received,
        boost::asio::streambuf& input) const
{
    // Yaaaaay brackets!
    if (not error) {
        RpbGetResp response;
        if (message::retrieve(response, bytes_received, input)) {
            boost::unique_lock<boost::mutex> protect(mutex_);
            if (not (response.has_unchanged() and response.unchanged())) {
                if (response.has_vclock()) {
                    cached_siblings_ = response.content();
                    cached_vector_clock_ = response.vclock();
                    p->set_value(cached_siblings_);
                } else {
                    p->set_value(boost::optional<object::siblings>());
                }
            } else {
                p->set_value(cached_siblings_);
            }
        } else {
            p->set_exception(boost::copy_exception(
                    std::invalid_argument("Riak server's response was nonsensical.")));
        }
    } else {
        p->set_exception(boost::copy_exception(boost::system::system_error(error)));
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
