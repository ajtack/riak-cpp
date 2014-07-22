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


class client::request_runner
      : public std::enable_shared_from_this<client::request_runner>
{
  public:
    typedef request_runner self;

    request_runner (client& client, const application_request_context&& application_context)
      : client_(client)
      , request_context_(std::move(application_context))
    {   }

    application_request_context::automatic_record_ostream<decltype(client::log_)> log (
            riak::log::severity sev = riak::log::severity::info )
    {
        return request_context_.log(client_.log_, sev);
    }

    typedef std::function<message::handler(const std::shared_ptr<object>&)> resolution_response_handler_factory;

    #define KV_COMMON_PARAMS const key& bucket, const key& k, sibling_resolution&

    void run_get_request (KV_COMMON_PARAMS, get_response_handler&);

    bool accept_get_response (KV_COMMON_PARAMS, get_response_handler,
            const std::error_code&, std::size_t, const std::string&);

    template <typename ResponseType>
    void resolve_siblings_and_put (KV_COMMON_PARAMS, const ResponseType&, get_response_handler);

    void put_with_vclock (
            const key& bucket,
            const key& k,
            const boost::optional<vector_clock>&,
            const std::shared_ptr<object>&,
            put_response_handler);

    void put_resolved_sibling (
            const key& bucket,
            const key& k,
            const vector_clock&,
            const std::shared_ptr<object>&,
            resolution_response_handler_factory&);

    bool return_successfully_resolved_sibling_or_retry (
            KV_COMMON_PARAMS,
            std::shared_ptr<object>& successfully_resolved_sibling,
            get_response_handler,
            const std::error_code&,
            std::size_t,
            const std::string&);

    #undef KV_COMMON_PARAMS

    bool accept_put_response (
            put_response_handler respond_to_application,
            const std::error_code& error,
            std::size_t bytes_received,
            const std::string& data);

    bool accept_delete_response (
            delete_response_handler respond_to_application,
            const std::error_code& error,
            std::size_t bytes_received,
            const std::string& data);

  private:
    client& client_;
    const application_request_context request_context_;

    void send_put_request (const RpbPutReq&, message::handler);
};


client::client (
        const transport::delivery_provider&& d,
        const sibling_resolution&& sr,
        boost::asio::io_service& ios,
        const request_failure_parameters& fp,
        const object_access_parameters& ao)
  : deliver_request_(d),
    resolve_siblings_(sr),
    access_overrides_(ao),
    request_failure_defaults_(fp),
    ios_(ios)
{   }


void client::delete_object (const key& bucket, const key& k, delete_response_handler h)
{
    assert(this);
    assert(not bucket.empty());  // TODO: if (not bucket.empty) ... else ...
    assert(not k.empty());       // TODO: if (not key.empty) ... else ...

    application_request_context context(access_overrides_, request_failure_defaults_);
    context.log(log_) << "DELETE '" << bucket << "' / '" << k << '\'';

    auto runner = std::make_shared<request_runner>(*this, std::move(context));
    message::handler handle_whole_response = std::bind(&request_runner::accept_delete_response, runner, h, _1, _2, _3);

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
    auto handle_buffered_response = message::make_buffering_handler(handle_whole_response);
    auto wire_request = std::make_shared<request_with_timeout>(
            query.to_string(),
            request_failure_defaults_.response_timeout,
            handle_buffered_response,
            ios_);
    
    wire_request->dispatch_via(deliver_request_);
}


bool client::request_runner::accept_delete_response (
        delete_response_handler respond_to_application,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    if (not error) {
        log(log::severity::trace) << "Parsing server response ...";

        if (message::verify_code(message::code::DeleteResponse, bytes_received, data)) {
            log(log::severity::info) << "Delete successful.";
            respond_to_application(riak::make_error_code());
        } else {
            log(log::severity::error) << "Received something other than a DELETE reply (parsing failed).";
            respond_to_application(riak::make_error_code(communication_failure::unparseable_response));
        }
    } else {
        log(log::severity::error) << "Request failed: " << error.message();
        respond_to_application(error);
    }

    // Always terminate the request, whether success or failure.
    return true;
}


void client::get_object (const key& bucket, const key& k, get_response_handler handle_get_result)
{
    assert(this);
    assert(not bucket.empty());  // TODO: if (not bucket.empty) ... else ...
    assert(not k.empty());       // TODO: if (not key.empty) ... else ...
    
    application_request_context context(access_overrides_, request_failure_defaults_);
    context.log(log_) << "GET '" << bucket << "' / '" << k << '\'';

    auto runner = std::make_shared<request_runner>(*this, std::move(context));
    runner->run_get_request(bucket, k, resolve_siblings_, handle_get_result);
}


void client::request_runner::run_get_request (
        const key& bucket,
        const key& k,
        sibling_resolution& resolve_siblings,
        get_response_handler& handle_get_result)
{
    RpbGetReq request;
    request.set_bucket(bucket);
    request.set_key(k);
    auto& overridden = request_context_.access_overrides;
    if (overridden.r )            request.set_r           (*overridden.r);
    if (overridden.pr)            request.set_pr          (*overridden.pr);
    if (overridden.basic_quorum)  request.set_basic_quorum(*overridden.basic_quorum);
    if (overridden.notfound_ok)   request.set_notfound_ok (*overridden.notfound_ok);
    request.clear_if_modified();
    request.set_head(false);
    request.set_deletedvclock(true);
    auto query = message::encode(request);

    message::handler handle_whole_response =
            std::bind(&request_runner::accept_get_response,
                    shared_from_this(),
                    bucket, k,
                    resolve_siblings,
                    handle_get_result,
                    /* error */ _1, /* data size */ _2, /* data */ _3);
    auto handle_buffered_response = message::make_buffering_handler(handle_whole_response);
    auto wire_request = std::make_shared<request_with_timeout>(
            query.to_string(),
            request_context_.request_failure_defaults.response_timeout,
            handle_buffered_response,
            client_.ios_);
    
    wire_request->dispatch_via(client_.deliver_request_);
}

//=============================================================================
    namespace {
//=============================================================================

RpbPutReq basic_put_request_for (
        const key& bucket,
        const key& k,
        const std::shared_ptr<object>& content,
        const application_request_context&);

typedef std::function <bool(std::shared_ptr<object>&,
                            const std::error_code&,
                            std::size_t,
                            const std::string&)>
        resolution_response_handler_for_object;

message::handler make_resolution_response_handler (
        const std::shared_ptr<object>& resolved_sibling,
        resolution_response_handler_for_object boilerplate)
{
    return std::bind(boilerplate, resolved_sibling, _1, _2, _3);
}

//=============================================================================
    }   //   namespace (anonymous)
//=============================================================================

bool client::request_runner::accept_get_response (
        const key& bucket,
        const key& k,
        sibling_resolution& resolve_siblings,
        get_response_handler respond_to_application,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    // A possible response in several cases.
    std::shared_ptr<object> no_content;
    value_updater no_value_updater;
    std::function<void(const std::error_code&)> report_error = std::bind(respond_to_application, _1, no_content, no_value_updater);

    if (not error) {
        log(log::severity::trace) << "Parsing server response ...";
        assert(bytes_received != 0);
        assert(bytes_received == data.size());

        RpbGetResp response;
        if (message::retrieve(response, data.size(), data)) {
            auto& old_context = request_context_;
            auto new_request_runner = std::make_shared<request_runner>(client_, old_context.copy_with_new_request_id());
            value_updater add_content = std::bind(&self::put_with_vclock, new_request_runner,
                    bucket, k, boost::none, _1 /* object */, _2 /* response handler */);

            if (response.content_size() > 1) {
                if (response.has_vclock()) {
                    log(log::severity::trace) << "Found " << response.content_size() << " siblings; attempting resolution.";
                    resolve_siblings_and_put(bucket, k, resolve_siblings, response, respond_to_application);
                } else {
                    log(log::severity::error) << "Found " << response.content_size() << " siblings with no vector clock -- cannot resolve.";
                    report_error(communication_failure::inappropriate_response_content);
                }
            } else if (response.content_size() == 1) {
                std::shared_ptr<object> the_value(response.mutable_content()->ReleaseLast());

                if (response.has_vclock()) {
                    log(log::severity::info) << "GET successful (found object).";
                    value_updater update_content = std::bind(&self::put_with_vclock, new_request_runner,
                            bucket, k, response.vclock(), _1 /* object */, _2 /* response handler */);
                    respond_to_application(riak::make_error_code(), the_value, update_content);
                } else {
                    log(log::severity::warning) << "Found 1 object with no vector clock -- storing to this index may create siblings.";
                    respond_to_application(
                            riak::make_error_code(communication_failure::missing_vector_clock),
                            the_value,
                            add_content /* Creates a sibling, probably. */);
                }
            } else {
                log(log::severity::info) << "GET successful (no content).";
                respond_to_application(riak::make_error_code(), no_content, add_content);
            }
        } else {
            log(log::severity::error) << "Received a reply from the server that could not be decoded.";
            report_error(communication_failure::unparseable_response);
        }
    } else {
        log(log::severity::error) << "Request failed: " << error.message();
        report_error(error);
    }

    // Always terminate the request, whether success or failure.
    return true;
}


template <typename ResponseType>
void client::request_runner::resolve_siblings_and_put (
        const key& bucket,
        const key& k,
        sibling_resolution& resolve_siblings,
        const ResponseType& response,
        get_response_handler respond_to_application)
{
    assert(response.content_size() > 1);
    auto resolved_content = resolve_siblings(response.content());

    if (!! resolved_content) {
        log(log::severity::trace) << "Resolved value has vector clock '" << response.vclock() << "'. Transmitting ...";
        resolution_response_handler_for_object handle_resolved_sibling_put_response =
                std::bind(&self::return_successfully_resolved_sibling_or_retry, shared_from_this(),
                        bucket, k, resolve_siblings, _1, respond_to_application,
                        _2, _3, _4);
        resolution_response_handler_factory deliver_resolved_sibling =
                std::bind(make_resolution_response_handler, _1, handle_resolved_sibling_put_response);

        put_resolved_sibling(bucket, k, response.vclock(), resolved_content, deliver_resolved_sibling);
    } else {
        log(log::severity::warning) << "Sibling resolution yielded NULL. Responding as for 'no content'.";
        auto& no_content = resolved_content;
        auto& old_context = request_context_;
        auto new_request_runner = std::make_shared<request_runner>(client_, old_context.copy_with_new_request_id());
        value_updater add_content = std::bind(&self::put_with_vclock, new_request_runner,
                bucket, k, boost::none, _1 /* object */, _2 /* response handler */);

        respond_to_application(riak::make_error_code(), no_content, add_content);
    }
}


bool client::request_runner::return_successfully_resolved_sibling_or_retry (
        const key& bucket,
        const key& k,
        sibling_resolution& resolve_siblings,
        std::shared_ptr<object>& cached_object,
        get_response_handler respond_to_application,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    std::shared_ptr<object> no_content;
    value_updater add_sibling = std::bind(&self::put_with_vclock, shared_from_this(),
            bucket, k, boost::none, /* object */ _1, /* put resp */ _2);

    if (not error) {
        log(log::severity::trace) << "Processing result of sibling resolution ...";
        assert(bytes_received != 0);
        assert(bytes_received == data.size());
        
        RpbPutResp response;
        if (message::retrieve(response, data.size(), data)) {
            if (response.content_size() == 1 and response.has_vclock()) {
                value_updater put_new_value = std::bind(&self::put_with_vclock, shared_from_this(),
                        bucket, k, response.vclock(),
                        _1 /* new value */, _2 /* response_handler */);

                log(log::severity::info) << "GET successful after applying sibling resolution.";
                respond_to_application(riak::make_error_code(), cached_object, put_new_value);
            } else {
                log(log::severity::trace) << "Value collided again upon resolution. Fetching new siblings ...";
                run_get_request(bucket, k, resolve_siblings, respond_to_application);
            }
        } else {
            log(log::severity::error) << "Sibling resolution was interrupted by an unusable response from the server.";
            auto nonsense = riak::make_error_code(communication_failure::unparseable_response);
            respond_to_application(nonsense, no_content, add_sibling);
        }
    } else {
        log(log::severity::error) << "Sibling resolution was interrupted by a network failure: \"" << error.message() << "\"";
        respond_to_application(error, no_content, add_sibling);
    }

    // Always terminate the request, whether success or failure.
    return true;
}


void client::request_runner::put_with_vclock (
        const key& bucket,
        const key& k,
        const boost::optional<vector_clock>& vclock,
        const std::shared_ptr<object>& content,
        put_response_handler application_response)
{
    log(log::severity::info) << "PUT '" << bucket << "' / '" << k << '\'';
    RpbPutReq request = basic_put_request_for(bucket, k, content, request_context_);
    if (!! vclock) {
        log(log::severity::trace) << "New value has vector clock '" << *vclock << '\'';
        request.set_vclock(*vclock);
    } else {
        log(log::severity::trace) << "Putting new value (no ancestor).";
    }

    request.set_return_body(false);
    request.set_if_not_modified(false);
    request.set_if_none_match(false);
    request.set_return_head(false);

    auto handle_response = std::bind(&self::accept_put_response, shared_from_this(),
            application_response, /* error */ _1, /* size */ _2, /* payload */ _3);
    send_put_request(request, handle_response);
}


void client::request_runner::put_resolved_sibling (
        const key& bucket,
        const key& k,
        const vector_clock& vclock,
        const std::shared_ptr<object>& content,
        resolution_response_handler_factory& response_handler_for)
{
    RpbPutReq request = basic_put_request_for(bucket, k, content, request_context_);
    request.set_vclock(vclock);
    request.set_return_body(false);
    request.set_if_not_modified(false);
    request.set_if_none_match(false);
    request.set_return_head(true);     // <-- Different from a regular put -- we don't need to know.
    send_put_request(request, response_handler_for(content));
}


void client::request_runner::send_put_request (const RpbPutReq& r, message::handler handle_whole_put_response)
{
    auto query = message::encode(r);
    auto handle_buffered_put_response = message::make_buffering_handler(handle_whole_put_response);
    auto wire_request = std::make_shared<request_with_timeout>(
            query.to_string(),
            request_context_.request_failure_defaults.response_timeout,
            handle_buffered_put_response,
            client_.ios_);
    
    wire_request->dispatch_via(client_.deliver_request_);
}


bool client::request_runner::accept_put_response (
        put_response_handler respond_to_application,
        const std::error_code& error,
        std::size_t bytes_received,
        const std::string& data)
{
    if (not error) {
        log(log::severity::trace) << "Parsing server response ...";

        RpbPutResp response;
        if (message::retrieve(response, data.size(), data)) {
            log(log::severity::info) << "PUT successful.";
            respond_to_application(riak::make_error_code());
        } else {
            log(log::severity::error) << "Received something other than a PUT reply (parsing failed).";
            respond_to_application(riak::make_error_code(communication_failure::unparseable_response));
        }
    } else {
        log(log::severity::error) << "Request failed: " << error.message();
        respond_to_application(error);
    }

    // Always terminate the request, whether success or failure.
    return true;
}

//=============================================================================
    namespace {
//=============================================================================

RpbPutReq basic_put_request_for (
        const key& bucket,
        const key& k,
        const std::shared_ptr<object>& content,
        const application_request_context& context)
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

//=============================================================================
    }   // namespace (anonymous)
}   // namespace riak
//=============================================================================
