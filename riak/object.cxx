/*!
 * \file
 * Implements asynchronous communication with a Riak server over objects.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#include <boost/thread/mutex.hpp>
#include <riak/bucket.hxx>
#include <riak/message.hxx>
#include <riak/object.hxx>
#include <riak/response.hxx>
#include <riak/riakclient.pb.h>
#include <riak/store.hxx>
#include <vector>

//=============================================================================
namespace riak {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


boost::shared_future<void> object::put (const object::value& c)
        // const object_access_parameters& )
{
    std::shared_ptr<boost::promise<void>> promise(new boost::promise<void>());
    
    // Set up the appropriate callback.
    if (cached_vector_clock_) {
        put_with_cached_vector_clock(promise, c);
    } else {
        // Fetch vclock first
        // Callback then assigns new content and puts
        assert(false);   // not implemented
    }
    
    return promise->get_future();
}


boost::shared_future<boost::optional<object::siblings>> object::fetch () const
        // const object_access_parameters& p = store_.object_access_defaults() ) const
{
    RpbGetReq request;
    request.set_bucket(this->bucket_);
    request.set_key(this->key());
    auto& overridden = overridden_access_parameters_;
    if (overridden.r )            request.set_r           (*overridden_access_parameters_.r);
    if (overridden.pr)            request.set_pr          (*overridden_access_parameters_.pr);
    if (overridden.basic_quorum)  request.set_basic_quorum(*overridden_access_parameters_.basic_quorum);
    if (overridden.notfound_ok)   request.set_notfound_ok (*overridden_access_parameters_.notfound_ok);
    if (cached_vector_clock_) request.set_if_modified(*cached_vector_clock_);
    request.set_head(false);
    request.set_deletedvclock(true);
    auto query = message::encode(request);
    
    typedef boost::optional<object::siblings> optval;
    std::shared_ptr<boost::promise<optval>> promise(new boost::promise<optval>());
    auto buffer = std::make_shared<std::vector<unsigned char>>();
    store::response_handler callback = std::bind(&object::on_fetch_response, shared_from_this(), promise, buffer, _1, _2, _3);
    store_.transmit_request(query.to_string(), callback, default_request_failure_parameters_.response_timeout);
    return promise->get_future();
}

//=============================================================================
    namespace {
//=============================================================================

bool promise_fulfilled (
        std::shared_ptr<boost::promise<boost::optional<object::siblings>>>& p,
        const std::string& request,
        boost::optional<object::siblings>& cached_siblings,
        boost::optional<std::string>& cached_vector_clock,
        boost::mutex& mutex)
{
    RpbGetResp response;
    if (message::retrieve(response, request.size(), request)) {
        boost::unique_lock<boost::mutex> protect(mutex);
        if (not (response.has_unchanged() and response.unchanged())) {
            if (response.has_vclock()) {
                cached_siblings = response.content();
                cached_vector_clock = response.vclock();
                p->set_value(cached_siblings);
            } else {
                protect.unlock();
                p->set_value(boost::optional<object::siblings>());
            }
        } else {
            p->set_value(cached_siblings);
        }
    } else {
        p->set_exception(boost::copy_exception(
                std::invalid_argument("Riak server's response was nonsensical.")));
    }
    
    // If we received solk, we are done. Else, we are still done.
    return true;
}
        
//=============================================================================
    }
//=============================================================================

bool object::on_fetch_response (
        std::shared_ptr<boost::promise<boost::optional<object::siblings>>>& p,
        std::shared_ptr<std::vector<unsigned char>> buffer,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& input) const
{
    // Yaaaaay brackets!
    if (not error) {
        buffer->insert(buffer->end(), input.begin(), input.end());
        auto next_response = response::next_partial_response(buffer->begin(), buffer->end());
        bool matched_whole_request = (next_response != buffer->begin());
        
        if (matched_whole_request) {
            std::string one_request(buffer->begin(), next_response);
            buffer->erase(buffer->begin(), next_response);
            return promise_fulfilled(p, one_request, cached_siblings_, cached_vector_clock_, mutex_);
        } else {
            // Wait for more data!
            return false;
        }
    } else {
        p->set_exception(boost::copy_exception(std::system_error(error)));
        return true;
    }
}


void object::put_with_cached_vector_clock (
        std::shared_ptr<boost::promise<void>>& promise,
        const object::value& c)
{
    assert(!! cached_vector_clock_);
    
    RpbPutReq request;
    request.set_bucket(this->bucket_);
    request.set_key(this->key());
    request.set_vclock(*cached_vector_clock_);
    
    // TODO: Really?
    request.mutable_content()->CopyFrom(c);
    
    auto& overridden = overridden_access_parameters_;
    if (overridden.w )  request.set_w (*overridden.w );
    if (overridden.dw)  request.set_dw(*overridden.dw);
    if (overridden.pw)  request.set_pw(*overridden.pw);
    
    // TODO: How do we control these options without being corny?
    request.set_return_body(false);
    request.set_if_not_modified(false);
    request.set_if_none_match(false);
    request.set_return_head(false);
    
    auto query = message::encode(request);
    store::response_handler callback = std::bind(&object::on_put_response, shared_from_this(), promise, _1, _2, _3);
    store_.transmit_request(query.to_string(), callback, default_request_failure_parameters_.response_timeout);
}


bool object::on_put_response (
    std::shared_ptr<boost::promise<void>>& p,
    const std::error_code& error,
    std::size_t bytes_received,
    const std::string& input) const
{
    if (not error) {
        RpbPutResp response;
        if (message::retrieve(response, bytes_received, input)) {
            boost::unique_lock<boost::mutex> protect(mutex_);
            if (response.has_vclock()   )  cached_vector_clock_ = response.vclock();
            if (response.content_size() > 0)  cached_siblings_ = response.content();
            // TODO: We assume that key is NULL, because we had to have a key to get here.
            p->set_value();
        } else {
            p->set_exception(boost::copy_exception(
                    std::invalid_argument("Riak server's response was nonsensical.")));
        }
    } else {
        p->set_exception(boost::copy_exception(std::system_error(error)));
    }
    
    return true;
}

//=============================================================================
}   // namespace riak
//=============================================================================
