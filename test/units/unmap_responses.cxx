/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/units/riak-client-with-mocked-transport.hxx>
#include <system_error>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

TEST_F(riak_client_with_mocked_transport, client_survives_nonsense_reply_to_unmap)
{
    auto future = client->bucket("a").unmap("document");
    
    EXPECT_CALL(*closure_signal, exercise()).Times(0);
    std::string garbage("uhetnaoutaenosueosaueoas");
    request_handler(std::error_code(), garbage.size(), garbage);
    
    // The above in binary would encode a much longer request: this request would eventually time out.
    ASSERT_TRUE(not future.is_ready());
}


TEST_F(riak_client_with_mocked_transport, client_survives_extra_RpbDelResp_from_unmap)
{
    auto future = client->bucket("a").unmap("document");

    EXPECT_CALL(*closure_signal, exercise());
    riak::message::wire_package long_reply(riak::message::code::DeleteResponse, "atnhueoauheas(garbage)");
    request_handler(std::error_code(), long_reply.to_string().size(), long_reply.to_string());
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}


TEST_F(riak_client_with_mocked_transport, client_accepts_well_formed_RbpDelResp)
{
    auto future = client->bucket("a").unmap("document");
    
    EXPECT_CALL(*closure_signal, exercise());
    riak::message::wire_package clean_reply(riak::message::code::DeleteResponse, "");
    request_handler(std::error_code(), clean_reply.to_string().size(), clean_reply.to_string());
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}


TEST_F(riak_client_with_mocked_transport, client_accepts_well_formed_unmap_response_in_parts)
{
    auto future = client->bucket("a").unmap("document");
    
    std::string response_data;
    std::string canned_delete_response = "";   // Deletes are only message code responses.
    riak::message::wire_package clean_reply(riak::message::code::DeleteResponse, canned_delete_response);
    auto data = clean_reply.to_string();
    assert(data.size() >= 2);
    auto first_half = data.substr(0, data.size() / 2);
    auto second_half = data.substr(data.size() / 2, data.size() - first_half.size());
    request_handler(std::error_code(), first_half.size(), first_half);
    ASSERT_TRUE(not future.is_ready());

    EXPECT_CALL(*closure_signal, exercise());
    request_handler(std::error_code(), second_half.size(), second_half);
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
