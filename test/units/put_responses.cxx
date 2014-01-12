/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/fixtures/get_and_put_client.hxx>
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

//=============================================================================
        }   // namespace (anonymous)
//=============================================================================

TEST_F(get_and_put_client, put_for_cold_object_with_well_formed_response_is_successful)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
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
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::no_error))));
    EXPECT_CALL(close_request_2, exercise()).Times(1);
    request_handler_2(std::error_code(), clean_put_reply().size(), clean_put_reply());
}


TEST_F(get_and_put_client, client_survives_long_nonsense_reply_to_cold_put)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
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
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::no_error)))).Times(0);
    EXPECT_CALL(close_request_2, exercise()).Times(0);
    request_handler_2(std::error_code(), garbage.size(), garbage);
    
}


TEST_F(get_and_put_client, client_survives_extra_data_in_cold_put_response)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
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
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::no_error))));
    EXPECT_CALL(close_request_2, exercise());
    request_handler_2(std::error_code(), long_reply.size(), long_reply);
}


TEST_F(get_and_put_client, client_accepts_well_formed_put_response_in_parts)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
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
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::no_error)))).Times(0);
    EXPECT_CALL(close_request_2, exercise()).Times(0);
    request_handler_2(std::error_code(), first_half.size(), first_half);

    // ... part 2
    EXPECT_CALL(put_response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::no_error))));
    EXPECT_CALL(close_request_2, exercise());
    request_handler_2(std::error_code(), second_half.size(), second_half);
}


TEST_F(get_and_put_client, client_correctly_delivers_put_reply_with_vector_clock)
{
    ::riak::value_updater update_value;
    client.get_object("a", "document", get_response_handler);

    // Build a response with a value and vector clock
    RpbGetResp original_get_response;
    original_get_response.set_vclock("Something!");
    RpbContent* data = original_get_response.add_content();
    data->set_value("Whatever");
    std::string content_laden_reply_data;
    original_get_response.SerializeToString(&content_laden_reply_data);
    riak::message::wire_package content_laden_reply(riak::message::code::GetResponse, content_laden_reply_data);

    // Respond to the read-before-write GET.
    EXPECT_CALL(get_response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
            Pointee(Property(&riak::object::value, StrEq(data->value()))),
            _))
        .WillOnce(SaveArg<2>(&update_value));
    EXPECT_CALL(close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), content_laden_reply.to_string().size(), content_laden_reply.to_string());

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

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
