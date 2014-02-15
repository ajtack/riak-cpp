/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <boost/uuid/uuid_io.hpp>
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/fixtures/get_and_put_client.hxx>
#include <test/matchers/has_attribute.hxx>
#include <test/matchers/log_record_attribute_set.hxx>
#include <system_error>

using namespace ::testing;
using riak::test::fixture::get_and_put_client;

//=============================================================================
namespace riak {
    namespace test {
        namespace {
//=============================================================================

const std::string clean_fetch_reply ()
{
    std::string fetch_response;
    RpbGetResp().SerializeToString(&fetch_response);
    riak::message::wire_package correct_reply(riak::message::code::GetResponse, fetch_response);
    return correct_reply.to_string();
}


const std::string clean_put_reply ()
{
    RpbPutResp put_response;
    std::string response_with_extra;
    RpbGetResp().SerializeToString(&response_with_extra);
    riak::message::wire_package correct_reply(riak::message::code::PutResponse, response_with_extra);
    return correct_reply.to_string();
}


/*! Yields a GET response message with exactly one value (and accompanying vector clock). */
RpbGetResp content_laden_fetch_reply ()
{
    RpbGetResp reply;
    reply.set_vclock("Something!");
    RpbContent* data = reply.add_content();
    data->set_value("Whatever");
    return reply;
}


template <typename ProtocolBufferType>
std::string as_wire_request_data (const ProtocolBufferType& message, riak::message::code message_code)
{
    std::string message_data;
    message.SerializeToString(&message_data);
    return riak::message::wire_package(message_code, message_data).to_string();
}

//=============================================================================
        }   // namespace (anonymous)
//=============================================================================

TEST_F(get_and_put_client, client_receives_socket_errors_from_object_update)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_error_code()),
            Eq(std::shared_ptr<riak::object>()),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    auto val = std::make_shared<object>();
    val->set_value("balooooooga!");
    update_value(val, put_response_handler);


    // Require at least one error line in logs.
    using riak::log::severity;
    EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute<severity>("Severity", Eq(severity::error)))));    

    // Respond to the PUT server.
    EXPECT_CALL(put_response_handler_mock, execute(Eq(std::make_error_code(std::errc::connection_reset))));
    EXPECT_CALL(close_request_2, exercise()).Times(1);
    std::string garbage("uhetnaoutaenosueosaueoas");
    request_handler_2(std::make_error_code(std::errc::connection_reset), garbage.size(), garbage);
}


TEST_F(get_and_put_client, client_survives_wrong_code_reply_upon_object_update)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_error_code()),
            Eq(std::shared_ptr<riak::object>()),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    auto val = std::make_shared<object>();
    val->set_value("balooooooga!");
    update_value(val, put_response_handler);

    // Require at least one error line in logs -- this is highly irregular.
    using riak::log::severity;
    EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute<severity>("Severity", Eq(severity::error)))));
    
    // Respond to the PUT server.
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_error_code(communication_failure::unparseable_response))));
    EXPECT_CALL(close_request_2, exercise()).Times(1);
    riak::message::wire_package bad_reply(riak::message::code::GetResponse /* should be put */, "something");
    request_handler_2(riak::make_error_code(), bad_reply.to_string().size(), bad_reply.to_string());
}


TEST_F(get_and_put_client, empty_server_replies_to_object_update_are_buffered)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_error_code()),
            Eq(std::shared_ptr<riak::object>()),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    auto val = std::make_shared<object>();
    val->set_value("balooooooga!");
    update_value(val, put_response_handler);

    // Respond to the PUT server.
    riak::message::wire_package bad_reply(riak::message::code::GetResponse /* should be put */, "something");
    request_handler_2(riak::make_error_code(), 0, "");
}


TEST_F(get_and_put_client, put_for_cold_object_with_well_formed_response_is_successful)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_error_code()),
            Eq(std::shared_ptr<riak::object>()),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    auto val = std::make_shared<object>();
    val->set_value("balooooooga!");
    update_value(val, put_response_handler);

    // Respond from server.
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_error_code())));
    EXPECT_CALL(close_request_2, exercise()).Times(1);
    request_handler_2(std::error_code(), clean_put_reply().size(), clean_put_reply());
}


TEST_F(get_and_put_client, client_survives_long_nonsense_reply_to_cold_put)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_error_code()),
            Eq(std::shared_ptr<riak::object>()),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    auto val = std::make_shared<object>();
    val->set_value("balooooooga!");
    update_value(val, put_response_handler);

    // Respond from server. The below would encode a much longer request: this request would eventually time out.
    std::string garbage("uhetnaoutaenosueosaueoas");
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_error_code()))).Times(0);
    EXPECT_CALL(close_request_2, exercise()).Times(0);
    request_handler_2(std::error_code(), garbage.size(), garbage);
    
}


TEST_F(get_and_put_client, client_survives_extra_data_in_cold_put_response)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_error_code()),
            Eq(std::shared_ptr<riak::object>()),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    auto val = std::make_shared<object>();
    val->set_value("balooooooga!");
    update_value(val, put_response_handler);

    // Respond from server.
    std::string long_reply = clean_put_reply() + "aueoauseonsauenats";
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_error_code())));
    EXPECT_CALL(close_request_2, exercise());
    request_handler_2(std::error_code(), long_reply.size(), long_reply);
}


TEST_F(get_and_put_client, client_accepts_well_formed_put_response_in_parts)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_error_code()),
            Eq(std::shared_ptr<riak::object>()),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response from client.
    auto val = std::make_shared<object>();
    val->set_value("balooooooga!");
    update_value(val, put_response_handler);

    // Proceed with the two-part server's reply to Put
    auto data = clean_put_reply();
    assert(data.size() >= 2);
    auto first_half = data.substr(0, data.size() / 2);
    auto second_half = data.substr(data.size() / 2, data.size() - first_half.size());

    // ... part 1
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_error_code()))).Times(0);
    EXPECT_CALL(close_request_2, exercise()).Times(0);
    request_handler_2(std::error_code(), first_half.size(), first_half);

    // ... part 2
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_error_code())));
    EXPECT_CALL(close_request_2, exercise());
    request_handler_2(std::error_code(), second_half.size(), second_half);
}


TEST_F(get_and_put_client, client_correctly_delivers_put_reply_with_vector_clock)
{
    ::riak::value_updater update_value;
    
    // The GET reply distinguishing this case.
    const auto original_get_response = content_laden_fetch_reply();

    // Respond to the read-before-write GET.
    client.get_object("a", "document", get_response_handler);
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_error_code()),
            Pointee(Property(&riak::object::value, StrEq(original_get_response.content(0).value()))),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    const auto content_laden_reply_data = as_wire_request_data(original_get_response, riak::message::code::GetResponse);
    request_handler_1(std::error_code(), content_laden_reply_data.size(), content_laden_reply_data);

    // Proceed with PUT response from client.
    const std::string& put_request_to_server = received_request_2;
    auto val = std::make_shared<object>();
    val->set_value("balooooooga!");
    update_value(val, put_response_handler);

    // Check that the following PUT contained the expected sibling.
    if (not put_request_to_server.empty()) {
        RpbPutReq put_request;
        if (message::retrieve(put_request, put_request_to_server.size(), put_request_to_server)) {
            ASSERT_TRUE(put_request.has_vclock());
            ASSERT_EQ(original_get_response.vclock(), put_request.vclock());
        } else {
            ADD_FAILURE() << "Updating a value produced something other than a PUT request!";
        }
    } else {
        ADD_FAILURE() << "The update request did not reach the server!";
    }
}


TEST_F(get_and_put_client, cold_object_update_after_get_gets_new_application_request_id)
{
    using riak::log::request_id_type;
    ::riak::value_updater update_value;

    // Capture the first log record from the GET portion of the request.
    boost::log::record_view original_get_log_record;
    EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute("Riak/ClientRequestId", A<request_id_type>()))))
        .WillOnce(SaveArg<0>(&original_get_log_record))
        .RetiresOnSaturation();
    client.get_object("a", "document", get_response_handler);

    // Respond to the GET.
    EXPECT_CALL(get_response_handler_mock, execute(Eq(riak::make_error_code()), _, _))
            .Times(AnyNumber())
            .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise())
            .Times(AnyNumber());
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Update the value: check that this yields a new request ID.
    const auto& original_record_attributes = original_get_log_record.attribute_values();
    const auto old_request_id = original_record_attributes["Riak/ClientRequestId"].extract<request_id_type>();
    EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(
                HasAttribute<request_id_type>("Riak/ClientRequestId", Ne(old_request_id)))))
            .Times(AtLeast(1));

    auto val = std::make_shared<object>();
    val->set_value("tuhetueo");
    ON_CALL(put_response_handler_mock, execute(Eq(riak::make_error_code()))).WillByDefault(Return());
    ON_CALL(close_request_2, exercise()).WillByDefault(Return());
    update_value(val, put_response_handler);
}


TEST_F(get_and_put_client, warm_object_update_after_get_gets_new_application_request_id)
{
    using riak::log::request_id_type;
    ::riak::value_updater update_value;

    // !!! Take a reply with actual data and a vector clock.
    //
    const auto fetch_reply = content_laden_fetch_reply();

    // Capture the first log record from the GET portion of the request.
    boost::log::record_view original_get_log_record;
    EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute("Riak/ClientRequestId", A<request_id_type>()))))
        .WillOnce(SaveArg<0>(&original_get_log_record))
        .RetiresOnSaturation();
    client.get_object("a", "document", get_response_handler);

    // Respond to the GET.
    EXPECT_CALL(get_response_handler_mock, execute(Eq(riak::make_error_code()), _, _))
            .Times(AnyNumber())
            .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise())
            .Times(AnyNumber());
    const auto fetch_reply_data = as_wire_request_data(fetch_reply, riak::message::code::GetResponse);
    request_handler_1(std::error_code(), fetch_reply_data.size(), fetch_reply_data);

    // Update the value: check that this yields a new request ID.
    const auto& original_record_attributes = original_get_log_record.attribute_values();
    const auto old_request_id = original_record_attributes["Riak/ClientRequestId"].extract<request_id_type>();
    EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(
                HasAttribute<request_id_type>("Riak/ClientRequestId", Ne(old_request_id)))))
            .Times(AtLeast(1));

    auto val = std::make_shared<object>();
    val->set_value("tuhetueo");
    ON_CALL(put_response_handler_mock, execute(Eq(riak::make_error_code()))).WillByDefault(Return());
    ON_CALL(close_request_2, exercise()).WillByDefault(Return());
    update_value(val, put_response_handler);
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
