/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/mocks/delete_request.hxx>
#include <test/units/riak-client-with-mocked-transport.hxx>
#include <system_error>

using namespace ::testing;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

TEST_F(riak_client_with_mocked_transport, client_survives_nonsense_reply_to_unmap)
{
    // In summary: mock a result handling function so we can tell when it was called.
    typedef mock::delete_request::result_handler result_handler_t;
    auto result_handler_mock = std::make_shared<result_handler_t>();
    auto result_handler = std::bind(&result_handler_t::execute, result_handler_mock, _1, _2, _3);

    client->delete_object("a", "document", result_handler);

    // Expect no calls, as this particular garbage suggests a longer reply; the request
    // would eventually time out.
    EXPECT_CALL(closure_signal, exercise()).Times(0);
    EXPECT_CALL(*result_handler_mock, execute(_, _, _)).Times(0);
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    std::string garbage("uhetnaoutaenosueosaueoas");
    request_handler(std::error_code(), garbage.size(), garbage);
}


TEST_F(riak_client_with_mocked_transport, client_survives_wrong_code_reply_to_unmap)
{
    // In summary: mock a result handling function so we can tell when it was called.
    typedef mock::delete_request::result_handler result_handler_t;
    auto result_handler_mock = std::make_shared<result_handler_t>();
    auto result_handler = std::bind(&result_handler_t::execute, result_handler_mock, _1, _2, _3);
    
    client->delete_object("a", "document", result_handler);
    
    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(*result_handler_mock, execute(Eq(riak::make_server_error(riak::errc::response_was_nonsense)), Eq("a"), Eq("document")));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    riak::message::wire_package bad_reply(riak::message::code::GetResponse, "whatever");
    request_handler(std::error_code(), bad_reply.to_string().size(), bad_reply.to_string());
}


TEST_F(riak_client_with_mocked_transport, client_survives_trailing_data_with_RpbDelResp)
{
    // In summary: mock a result handling function so we can tell when it was called.
    typedef mock::delete_request::result_handler result_handler_t;
    auto result_handler_mock = std::make_shared<result_handler_t>();
    auto result_handler = std::bind(&result_handler_t::execute, result_handler_mock, _1, _2, _3);

    client->delete_object("a", "document", result_handler);

    EXPECT_CALL(closure_signal, exercise());
    EXPECT_CALL(*result_handler_mock, execute(_, _, _));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(0);
    riak::message::wire_package long_reply(riak::message::code::DeleteResponse, "atnhueoauheas(garbage)");
    request_handler(std::error_code(), long_reply.to_string().size(), long_reply.to_string());
}


// TEST_F(riak_client_with_mocked_transport, client_accepts_well_formed_RbpDelResp)
// {
//     auto future = client->bucket("a").unmap("document");
    
//     EXPECT_CALL(closure_signal, exercise());
//     riak::message::wire_package clean_reply(riak::message::code::DeleteResponse, "");
//     request_handler(std::error_code(), clean_reply.to_string().size(), clean_reply.to_string());
//     ASSERT_TRUE(future.is_ready());
//     ASSERT_TRUE(future.has_value());
// }


// TEST_F(riak_client_with_mocked_transport, client_accepts_well_formed_unmap_response_in_parts)
// {
//     auto future = client->bucket("a").unmap("document");
    
//     std::string response_data;
//     std::string canned_delete_response = "";   // Deletes are only message code responses.
//     riak::message::wire_package clean_reply(riak::message::code::DeleteResponse, canned_delete_response);
//     auto data = clean_reply.to_string();
//     assert(data.size() >= 2);
//     auto first_half = data.substr(0, data.size() / 2);
//     auto second_half = data.substr(data.size() / 2, data.size() - first_half.size());
//     request_handler(std::error_code(), first_half.size(), first_half);
//     ASSERT_TRUE(not future.is_ready());

//     EXPECT_CALL(closure_signal, exercise());
//     request_handler(std::error_code(), second_half.size(), second_half);
//     ASSERT_TRUE(future.is_ready());
//     ASSERT_TRUE(future.has_value());
// }

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
