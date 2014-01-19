/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/fixtures/getting_client.hxx>
#include <test/matchers/has_attribute.hxx>
#include <test/matchers/log_record_attribute_set.hxx>
#include <system_error>

using namespace ::testing;
using riak::test::fixture::getting_client;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

RpbGetResp empty_get_response;

TEST_F(getting_client, client_survives_long_nonsense_reply_to_get)
{
    client->get_object("a", "document", response_handler);
    
    // Expect no calls, as this particular garbage suggests a longer reply; the request
    // would eventually time out.
    EXPECT_CALL(closure_signal, exercise()).Times(0);
    EXPECT_CALL(response_handler_mock, execute(_, _, _)).Times(0);
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    std::string garbage("uhetnaoutaenosueosaueoas");
    send_from_server(std::error_code(), garbage.size(), garbage);
}


TEST_F(getting_client, client_survives_wrong_code_reply_to_get)
{
    client->get_object("a", "document", response_handler);

    RpbGetResp nonempty_get_response;
    nonempty_get_response.add_content()->set_value("Son of a gun!");
    nonempty_get_response.set_vclock("whatever");
    std::string raw_response;
    nonempty_get_response.SerializeToString(&raw_response);
    riak::message::wire_package bad_reply(riak::message::code::PutResponse, raw_response);

    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::response_was_nonsense)),
            IsNull(),
            _));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);

    // Require at least one error line in logs -- this is highly irregular behavior.
    using riak::log::severity;
    EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute<severity>("Severity", Eq(severity::error)))));

    send_from_server(std::error_code(), bad_reply.to_string().size(), bad_reply.to_string());
}


TEST_F(getting_client, client_survives_extra_data_in_empty_get_response)
{
    client->get_object("a", "document", response_handler);

    std::string response_with_extra;
    empty_get_response.SerializeToString(&response_with_extra);
    riak::message::wire_package correct_reply(riak::message::code::GetResponse, response_with_extra);
    std::string long_reply = correct_reply.to_string() + "aueoauseonsauenats";

    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
            Eq(std::shared_ptr<riak::object>()),
            _));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    send_from_server(std::error_code(), long_reply.size(), long_reply);
}


TEST_F(getting_client, client_accepts_nonempty_get_response)
{
    client->get_object("a", "document", response_handler);

    RpbGetResp nonempty_get_response;
    nonempty_get_response.add_content()->set_value("Son of a gun!");
    nonempty_get_response.set_vclock("whatever");
    std::string raw_response;
    nonempty_get_response.SerializeToString(&raw_response);
    riak::message::wire_package nonempty_reply(riak::message::code::GetResponse, raw_response);

    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
            Pointee(Property(&riak::object::value, StrEq(nonempty_get_response.content(0).value()))),
            _));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    send_from_server(std::error_code(), nonempty_reply.to_string().size(), nonempty_reply.to_string());
}


TEST_F(getting_client, client_accepts_empty_RbpGetResp)
{
    client->get_object("a", "document", response_handler);
    
    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
            IsNull(),
            _));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    std::string response_data;
    empty_get_response.SerializeToString(&response_data);
    riak::message::wire_package clean_reply(riak::message::code::GetResponse, response_data);
    send_from_server(std::error_code(), clean_reply.to_string().size(), clean_reply.to_string());
}


TEST_F(getting_client, client_accepts_well_formed_response_in_parts)
{
    client->get_object("a", "document", response_handler);
    
    RpbGetResp nonempty_get_response;
    nonempty_get_response.add_content()->set_value("Son of a gun!");
    nonempty_get_response.set_vclock("whatever");
    std::string response_data;
    nonempty_get_response.SerializeToString(&response_data);
    riak::message::wire_package clean_reply(riak::message::code::GetResponse, response_data);
    auto data = clean_reply.to_string();
    assert(data.size() >= 2);
    auto first_half = data.substr(0, data.size() / 2);
    auto second_half = data.substr(data.size() / 2, data.size() - first_half.size());

    // The first half should be correctly buffered.
    EXPECT_CALL(closure_signal, exercise()).Times(0);
    EXPECT_CALL(response_handler_mock, execute(_, _, _)).Times(0);
    send_from_server(std::error_code(), first_half.size(), first_half);

    // The second half should trigger a response callback.
    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(
            Eq(riak::make_server_error(riak::errc::no_error)),
            Pointee(Property(&riak::object::value, StrEq(nonempty_get_response.content(0).value()))),
            _));
    send_from_server(std::error_code(), second_half.size(), second_half);
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
