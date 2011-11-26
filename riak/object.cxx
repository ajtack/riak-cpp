#include <boost/thread/mutex.hpp>
#include <riak/bucket.hxx>
#include <riak/message.hxx>
#include <riak/object.hxx>
#include <riak/riakclient.pb.h>
#include <riak/store.hxx>
#include <riak/utilities.hxx>
#include <system_error>

//=============================================================================
namespace riak {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


boost::shared_future<boost::optional<object::siblings>> object::fetch () const
{
    typedef boost::optional<object::siblings> optval;
    std::shared_ptr<boost::promise<optval>> promise(new boost::promise<optval>());
    
    // Type inference should work here, but it doesn't derive correctly. Strange.
    using std::function;
    function<void()> on_successful_fetch = std::bind(&fulfill_promise<optval>, promise, std::ref(cached_siblings_));
    function<void(const std::exception&)> on_failed_fetch = std::bind(&fail_promise<optval>, promise, _1);
    message::handler handle_whole_response =
            std::bind(&object::on_fetch_response,
                    shared_from_this(),
                    on_successful_fetch,
                    on_failed_fetch,
                    _1, _2, _3);
    
    auto handle_buffered_response = message::make_buffering_handler(handle_whole_response);
    fetch_to (handle_buffered_response);
    
    return promise->get_future();
}


boost::shared_future<void> object::put (const object::value& c)
{
    std::shared_ptr<boost::promise<void>> promise(new boost::promise<void>());
    
    // Here, we try to make sure that the client reads-before-writes, as per Basho client
    // library implementation guidebook recommendations.
    if (cache_is_hot_) {
        put_with_cached_vector_clock(promise, c);
    } else {
        using std::function;
        function<void()> on_successful_fetch =
                std::bind(&object::put_with_cached_vector_clock, shared_from_this(), promise, c);
        function<void(const std::exception&)> on_failed_fetch =
                std::bind(&fail_promise<void>, promise, _1);
        
        message::handler handle_fetch_response = 
                std::bind(&object::on_fetch_response,
                        shared_from_this(),
                        on_successful_fetch,
                        on_failed_fetch,
                        _1, _2, _3);
        
        auto handle_buffered_fetch_response = message::make_buffering_handler(handle_fetch_response);
        fetch_to (handle_buffered_fetch_response);
    }
    
    return promise->get_future();
}

//=============================================================================
    namespace {
//=============================================================================

void update_cache (
        const RpbGetResp& response,
        boost::optional<object::siblings>& cached_siblings,
        boost::optional<std::string>& cached_vector_clock,
        boost::mutex& mutex)
{
    boost::unique_lock<boost::mutex> protect(mutex);
    if (not (response.has_unchanged() and response.unchanged())) {
        if (response.has_vclock()) {
            cached_siblings = response.content();
            cached_vector_clock = response.vclock();
        }
    }
}
        
//=============================================================================
    }   // namespace (anonymous)
//=============================================================================

void object::fetch_to (message::buffering_handler& response_handler) const
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

    store_.transmit_request(
            query.to_string(), response_handler, default_request_failure_parameters_.response_timeout);
}


bool object::on_fetch_response (
        std::function<void()> proceed_with_next_step,
        std::function<void(const std::exception&)> fail,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& request) const
{
    // Note: This method assumes that it has been fed whole response objects. Anything less than
    // five bytes long is going to fail here in strange and wonderful ways. Use
    // message::make_buffering_handler!
    
    if (not error) {
        assert (bytes_received != 0);
        assert(bytes_received == request.size());
        
        RpbGetResp response;
        if (message::retrieve(response, request.size(), request)) {
            update_cache(response, cached_siblings_, cached_vector_clock_, mutex_);
            cache_is_hot_ = true;
            proceed_with_next_step();
        } else {
            fail(std::invalid_argument("Riak server's response was nonsensical."));
        }
    } else {
        fail(std::system_error(error));
    }
    
    // Whether we received a successful response or solk, we're done.
    return true;
}


void object::put_with_cached_vector_clock (
        std::shared_ptr<boost::promise<void>>& promise,
        const object::value& c)
{
    assert(!! cache_is_hot_);
    
    RpbPutReq request;
    request.set_bucket(this->bucket_);
    request.set_key(this->key());
    if (cached_vector_clock_)
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
    message::handler handle_whole_response =
            std::bind(&object::on_put_response, shared_from_this(), promise, _1, _2, _3);
    auto handle_buffered_put_response = message::make_buffering_handler(handle_whole_response);

    store_.transmit_request(
            query.to_string(),
            handle_buffered_put_response,
            default_request_failure_parameters_.response_timeout);
}


bool object::on_put_response (
    std::shared_ptr<boost::promise<void>>& p,
    const std::error_code& error,
    std::size_t bytes_received,
    const std::string& input) const
{
    // Note: This method assumes that it has been fed whole response objects. Anything less than
    // five bytes long is going to fail here in strange and wonderful ways. Use
    // message::make_buffering_handler!

    if (not error) {
        RpbPutResp response;
        if (message::retrieve(response, bytes_received, input)) {
            boost::unique_lock<boost::mutex> protect(mutex_);
            if (response.has_vclock()   )  cached_vector_clock_ = response.vclock();
            if (response.content_size() > 0)  cached_siblings_ = response.content();
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
