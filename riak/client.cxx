#include <riak/application_request_context.hxx>
#include <riak/client.hxx>
#include <riak/request_with_timeout.hxx>

//=============================================================================
namespace riak {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

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

//=============================================================================
    namespace {
//=============================================================================

bool accept_delete_response (
        delete_response_handler respond_to_application,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    if (not error) {
        if (message::verify_code(message::code::DeleteResponse, bytes_received, data))
            respond_to_application(riak::make_server_error());
        else
            respond_to_application(riak::make_server_error(riak::errc::response_was_nonsense));
    } else {
        respond_to_application(error);
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
    
    message::handler handle_whole_response = std::bind(&accept_delete_response, h, _1, _2, _3);
    auto handle_buffered_response = message::make_buffering_handler(handle_whole_response);
    auto wire_request = std::make_shared<request_with_timeout>(
            query.to_string(),
            request_failure_defaults_.response_timeout,
            handle_buffered_response,
            ios_);
    
    wire_request->dispatch_via(deliver_request_);
}

//=============================================================================
    namespace {
//=============================================================================

void run_get_request (
        const key& bucket,
        const key& k,
        sibling_resolution&,
        application_request_context&,
        get_response_handler&);

//=============================================================================
    }   // namespace (anonymous)
//=============================================================================

void client::get_object (const key& bucket, const key& k, get_response_handler handle_get_result)
{
    assert(this);
    assert(not bucket.empty());  // TODO: if (not bucket.empty) ... else ...
    assert(not k.empty());       // TODO: if (not key.empty) ... else ...
    
    application_request_context context(access_overrides_, request_failure_defaults_, deliver_request_, ios_);
    context.log(log_) << "GET " << bucket << " / " << k;
    run_get_request(bucket, k, resolve_siblings_, context, handle_get_result);
}

//=============================================================================
    namespace {
//=============================================================================

bool accept_get_response (
        const key& bucket,
        const key& k,
        sibling_resolution&,
        application_request_context&,
        get_response_handler,
        const std::error_code&,
        std::size_t,
        const std::string&);

void run_get_request (
        const key& bucket,
        const key& k,
        sibling_resolution& resolve_siblings,
        application_request_context& context,
        get_response_handler& handle_get_result)
{
    RpbGetReq request;
    request.set_bucket(bucket);
    request.set_key(k);
    auto& overridden = context.access_overrides;
    if (overridden.r )            request.set_r           (*overridden.r);
    if (overridden.pr)            request.set_pr          (*overridden.pr);
    if (overridden.basic_quorum)  request.set_basic_quorum(*overridden.basic_quorum);
    if (overridden.notfound_ok)   request.set_notfound_ok (*overridden.notfound_ok);
    request.clear_if_modified();
    request.set_head(false);
    request.set_deletedvclock(true);
    auto query = message::encode(request);

    message::handler handle_whole_response =
            std::bind(&accept_get_response,
                    bucket, k,
                    resolve_siblings,
                    context,
                    handle_get_result,
                    /* error */ _1, /* data size */ _2, /* data */ _3);
    auto handle_buffered_response = message::make_buffering_handler(handle_whole_response);
    auto wire_request = std::make_shared<request_with_timeout>(
            query.to_string(),
            context.request_failure_defaults.response_timeout,
            handle_buffered_response,
            context.ios);
    
    wire_request->dispatch_via(context.deliver_request);
}


typedef std::function <bool(std::shared_ptr<object>&,
                            const std::error_code&,
                            std::size_t,
                            const std::string&)>
        resolution_response_handler_for_object;

typedef std::function<message::handler(std::shared_ptr<object>&)> resolution_response_handler_factory;

bool retry_or_return_cached_value (
        const key& bucket,
        const key& k,
        std::shared_ptr<object>&,
        sibling_resolution&,
        get_response_handler,
        application_request_context&,
        const std::error_code&,
        std::size_t,
        const std::string&);

void put_cold (
        const key& bucket,
        const key& k,
        const std::shared_ptr<object>& content,
        application_request_context& context,
        put_response_handler& respond_to_application);

void put_resolved_sibling (
        const key& bucket,
        const key& k,
        std::shared_ptr<object>&,
        const vector_clock&,
        application_request_context& context,
        std::function<message::handler(std::shared_ptr<object>&)>&);

void put_with_vclock (
        const key& bucket,
        const key& k,
        const std::shared_ptr<object>&,
        const vector_clock&,
        application_request_context&,
        put_response_handler&);

RpbPutReq basic_put_request_for (
        const key& bucket,
        const key& k,
        const std::shared_ptr<object>& content,
        application_request_context&);

void send_put_request (RpbPutReq&, application_request_context&, message::handler);

bool parse_put_response (
        put_response_handler,
        const std::error_code&,
        std::size_t,
        const std::string&);


message::handler make_resolution_response_handler (
        std::shared_ptr<object>& resolved_sibling,
        resolution_response_handler_for_object boilerplate)
{
    return std::bind(boilerplate, resolved_sibling, _1, _2, _3);
}


template <typename ResponseType>
void resolve_siblings_and_put (
        const ResponseType& response,
        const key& bucket,
        const key& k,
        sibling_resolution& resolve_siblings,
        application_request_context& context,
        get_response_handler respond_to_application)
{
    assert(response.content_size() > 1);

    resolution_response_handler_for_object response_handler_for_object =
            std::bind(&retry_or_return_cached_value, bucket, k, _1, resolve_siblings, respond_to_application, context,
                    _2, _3, _4);
    resolution_response_handler_factory handler_factory =
            std::bind(make_resolution_response_handler, _1, response_handler_for_object);
    auto resolved_content = resolve_siblings(response.content());
    put_resolved_sibling(bucket, k, resolved_content, response.vclock(), context, handler_factory);
}


bool accept_get_response (
        const key& bucket,
        const key& k,
        sibling_resolution& resolve_siblings,
        application_request_context& context,
        get_response_handler respond_to_application,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    // A possible response in several cases.
    std::shared_ptr<object> no_content;
    value_updater no_value_updater;
    auto nonsense = riak::make_server_error(riak::errc::response_was_nonsense);
    auto tell_application_reply_was_nonsense = std::bind(respond_to_application, nonsense, no_content, no_value_updater);

    if (not error) {
        assert(bytes_received != 0);
        assert(bytes_received == data.size());
        
        RpbGetResp response;
        if (message::retrieve(response, data.size(), data)) {
            if (response.content_size() > 1) {
                if (response.has_vclock()) {
                    resolve_siblings_and_put(response, bucket, k, resolve_siblings, context, respond_to_application);
                } else {
                    tell_application_reply_was_nonsense();
                }
            } else if (response.content_size() == 1) {
                if (response.has_vclock()) {
                    value_updater update_content = std::bind(&put_with_vclock,
                            bucket, k, _1 /* object */,
                            response.vclock(),
                            context,
                            _2 /* response handler */);
                    std::shared_ptr<object> the_value(response.mutable_content()->ReleaseLast());
                    respond_to_application(riak::make_server_error(), the_value, update_content);
                } else {
                    tell_application_reply_was_nonsense();
                }
            } else {
                value_updater add_content =
                        std::bind(&put_cold,
                                bucket, k, _1 /* object */,
                                context,
                                _2 /* response handler */);
                respond_to_application(riak::make_server_error(), no_content, add_content);
            }
        } else {
            tell_application_reply_was_nonsense();
        }
    } else {
        respond_to_application(error, no_content, no_value_updater);
    }

    // Always terminate the request, whether success or failure.
    return true;
}


void put_cold (
        const key& bucket,
        const key& k,
        const std::shared_ptr<object>& content,
        application_request_context& context,
        put_response_handler& respond_to_application)
{
    RpbPutReq request = basic_put_request_for(bucket, k, content, context);
    request.set_return_body(false);
    request.set_if_not_modified(false);
    request.set_if_none_match(false);
    request.set_return_head(false);
    send_put_request(request, context, std::bind(&parse_put_response, respond_to_application, _1, _2, _3));
}


void put_resolved_sibling (
        const key& bucket,
        const key& k,
        std::shared_ptr<object>& content,
        const vector_clock& vclock,
        application_request_context& context,
        std::function<message::handler(std::shared_ptr<object>&)>& response_handler_for)
{
    RpbPutReq request = basic_put_request_for(bucket, k, content, context);
    request.set_vclock(vclock);
    request.set_return_body(false);
    request.set_if_not_modified(false);
    request.set_if_none_match(false);
    request.set_return_head(true);
    send_put_request(request, context, response_handler_for(content));
}


void put_with_vclock (
        const key& bucket,
        const key& k,
        const std::shared_ptr<object>& content,
        const vector_clock& vclock,
        application_request_context& context,
        put_response_handler& application_response)
{
    RpbPutReq request = basic_put_request_for(bucket, k, content, context);
    request.set_vclock(vclock);
    request.set_return_body(false);
    request.set_if_not_modified(false);
    request.set_if_none_match(false);
    request.set_return_head(false);
    send_put_request(
            request,
            context,
            std::bind(&parse_put_response, application_response, /* error */ _1, /* size */ _2, /* payload */ _3));
}


bool retry_or_return_cached_value (
        const key& bucket,
        const key& k,
        std::shared_ptr<object>& cached_object,
        sibling_resolution& resolve_siblings,
        get_response_handler respond_to_application,
        application_request_context& context,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    std::shared_ptr<object> no_content;
    value_updater add_sibling = std::bind(&put_cold, bucket, k, /* object */ _1, context, /* put resp */ _2);

    if (not error) {
        assert(bytes_received != 0);
        assert(bytes_received == data.size());
        
        RpbPutResp response;
        if (message::retrieve(response, data.size(), data)) {
            if (response.content_size() == 1 and response.has_vclock()) {
                value_updater put_new_value = std::bind(&put_with_vclock, bucket, k, _1, response.vclock(), context, _2);
                respond_to_application(riak::make_server_error(), cached_object, put_new_value);
            } else {
                run_get_request(bucket, k, resolve_siblings, context, respond_to_application);
            }
        } else {
            auto nonsense = riak::make_server_error(riak::errc::response_was_nonsense);
            respond_to_application(nonsense, no_content, add_sibling);
        }
    } else {
        respond_to_application(error, no_content, add_sibling);
    }

    // Always terminate the request, whether success or failure.
    return true;
}


RpbPutReq basic_put_request_for (
        const key& bucket,
        const key& k,
        const std::shared_ptr<object>& content,
        application_request_context& context)
{
    RpbPutReq request;
    request.set_bucket(bucket);
    request.set_key(k);
    request.mutable_content()->CopyFrom(*content);
    auto& overridden = context.access_overrides;
    if (overridden.w )  request.set_w (*overridden.w );
    if (overridden.dw)  request.set_dw(*overridden.dw);
    if (overridden.pw)  request.set_pw(*overridden.pw);
    return request;
}


void send_put_request (RpbPutReq& r, application_request_context& context, message::handler handle_whole_put_response)
{
    auto query = message::encode(r);
    auto handle_buffered_put_response = message::make_buffering_handler(handle_whole_put_response);
    auto wire_request = std::make_shared<request_with_timeout>(
            query.to_string(),
            context.request_failure_defaults.response_timeout,
            handle_buffered_put_response,
            context.ios);
    
    wire_request->dispatch_via(context.deliver_request);
}


bool parse_put_response (
        put_response_handler respond_to_application,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    if (not error) {
        assert(bytes_received != 0);
        assert(bytes_received == data.size());
        
        RpbPutResp response;
        if (message::retrieve(response, data.size(), data)) {
            respond_to_application(riak::make_server_error());
        } else {
            respond_to_application(riak::make_server_error(riak::errc::response_was_nonsense));
        }
    } else {
        respond_to_application(error);
    }

    // Always terminate the request, whether success or failure.
    return true;
}

//=============================================================================
    }   // namespace (anonymous)
}   // namespace riak
//=============================================================================
