/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/fixtures/deleting_client.hxx>
#include <test/matchers/has_attribute.hxx>
#include <test/matchers/log_record_attribute_set.hxx>
#include <system_error>

using namespace ::testing;
using riak::test::fixture::deleting_client;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

TEST_F(deleting_client, client_survives_nonsense_reply_to_unmap)
{
    client->delete_object("a", "document", response_handler);

    // Expect no calls, as this particular garbage suggests a longer reply; the request
    // would eventually time out.
    EXPECT_CALL(closure_signal, exercise()).Times(0);
    EXPECT_CALL(response_handler_mock, execute(_)).Times(0);
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    std::string garbage("uhetnaoutaenosueosaueoas");
    send_from_server(std::error_code(), garbage.size(), garbage);
}


TEST_F(deleting_client, client_survives_wrong_code_reply_to_unmap)
{
    client->delete_object("a", "document", response_handler);

    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::response_was_nonsense))));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);

    // Require at least one error line in logs -- this is highly irregular behavior.
    using riak::log::severity;
    EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute<severity>("Severity", Eq(severity::error)))));

    riak::message::wire_package bad_reply(riak::message::code::GetResponse, "whatever");
    send_from_server(std::error_code(), bad_reply.to_string().size(), bad_reply.to_string());
}


TEST_F(deleting_client, client_survives_trailing_data_with_RpbDelResp)
{
    client->delete_object("a", "document", response_handler);

    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::no_error))));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    riak::message::wire_package long_reply(riak::message::code::DeleteResponse, "atnhueoauheas(garbage)");
    send_from_server(std::error_code(), long_reply.to_string().size(), long_reply.to_string());
}


TEST_F(deleting_client, client_accepts_well_formed_RbpDelResp)
{
    client->delete_object("a", "document", response_handler);
    
    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::no_error))));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    riak::message::wire_package clean_reply(riak::message::code::DeleteResponse, "");
    send_from_server(std::error_code(), clean_reply.to_string().size(), clean_reply.to_string());

}


TEST_F(deleting_client, client_accepts_well_formed_unmap_response_in_parts)
{
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    client->delete_object("a", "document", response_handler);
    
    std::string response_data;
    std::string canned_delete_response = "";   // Deletes are only message code responses.
    riak::message::wire_package clean_reply(riak::message::code::DeleteResponse, canned_delete_response);
    auto data = clean_reply.to_string();
    assert(data.size() >= 2);
    auto first_half = data.substr(0, data.size() / 2);
    auto second_half = data.substr(data.size() / 2, data.size() - first_half.size());

    // The first half should be correctly buffered.
    EXPECT_CALL(closure_signal, exercise()).Times(0);
    EXPECT_CALL(response_handler_mock, execute(_)).Times(0);
    send_from_server(std::error_code(), first_half.size(), first_half);

    // The second half should trigger a response callback.
    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(response_handler_mock, execute(Eq(riak::make_server_error(riak::errc::no_error))));
    send_from_server(std::error_code(), second_half.size(), second_half);
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
