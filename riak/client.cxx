#include <riak/client.hxx>
#include <riak/request_with_timeout.hxx>

//=============================================================================
namespace riak {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// The default does not specify anything out of the ordinary with the configuration of the
// Riak servers themselves. The default is actually server-side.
const object_access_parameters client::access_override_defaults = object_access_parameters();


const request_failure_parameters client::failure_defaults = request_failure_parameters()
    .with_response_timeout(std::chrono::milliseconds(3000))
    .with_retries_permitted(1);


std::shared_ptr<client> make_client (
        transport::delivery_provider d,
        sibling_resolution sr,
        boost::asio::io_service& ios,
        const request_failure_parameters& failure_defaults,
        const object_access_parameters& access_override_defaults)
{
    std::shared_ptr<client> ptr(new client(d, sr, ios, failure_defaults, access_override_defaults));
    return ptr;
}


client::client (
        transport::delivery_provider& d,
        sibling_resolution sr,
        boost::asio::io_service& ios,
        const request_failure_parameters& fp,
        const object_access_parameters& ao)
  : deliver_request_(d),
    resolve_siblings_(sr),
    access_overrides_(ao),
    request_failure_defaults_(fp),
    ios_(ios)
{   }


bucket client::bucket (const key& k)
{
    assert(this);
    return bucket::bucket(deliver_request_, k, resolve_siblings_, request_failure_defaults_, access_overrides_);
}

//=============================================================================
    namespace {
//=============================================================================

bool on_server_response (
        delete_response_handler respond_to_application,
        const key& bucket,
        const key& k,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    if (not error) {
        if (message::verify_code(message::code::DeleteResponse, bytes_received, data))
            respond_to_application(riak::make_server_error(), bucket, k);
        else
            respond_to_application(riak::make_server_error(riak::errc::response_was_nonsense), bucket, k);
    } else {
        respond_to_application(error, bucket, k);
    }

    // Always terminate the request, whether success or failure.
    return true;
}

//=============================================================================
    }   // namespace (anonymous)
//=============================================================================

void client::delete_object (const key& bucket, const key& k, delete_response_handler h)
{
    assert(this);
    assert(not bucket.empty());  // TODO: if (not bucket.empty) ... else ...
    assert(not k.empty());       // TODO: if (not key.empty) ... else ...

    RpbDelReq request;
    request.set_bucket(bucket);
    request.set_key(k);
    auto& overridden = access_overrides_;
    if (overridden.r )   request.set_r (*overridden.r );
    if (overridden.rw)   request.set_rw(*overridden.rw);
    if (overridden.w )   request.set_w (*overridden.w );
    if (overridden.dw)   request.set_dw(*overridden.dw);
    if (overridden.pr)   request.set_pr(*overridden.pr);
    if (overridden.pw)   request.set_pw(*overridden.pw);
    auto query = message::encode(request);
    
    message::handler handle_whole_response = std::bind(&on_server_response, h, bucket, k, _1, _2, _3);
    auto handle_buffered_response = message::make_buffering_handler(handle_whole_response);
    auto wire_request = std::make_shared<request_with_timeout>(
            query.to_string(),
            request_failure_defaults_.response_timeout,
            handle_buffered_response,
            ios_);
    
    wire_request->dispatch_via(deliver_request_);
}

//=============================================================================
}   // namespace riak
//=============================================================================
