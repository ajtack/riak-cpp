/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/units/riak-client-mocked-for-two-requests.hxx>
#include <system_error>

using namespace ::testing;

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

TEST_F(riak_client_mocked_for_two_requests, client_survives_long_nonsense_reply_to_put)
{
    RpbContent val;
    val.set_value("balooooooga!");
    auto future = client->bucket("a")["document"]->put(val);

    // Respond to the read-before-write GET.
    EXPECT_CALL(*close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    std::string garbage("uhetnaoutaenosueosaueoas");
    request_handler_2(std::error_code(), garbage.size(), garbage);
    
    // The above in binary would encode a much longer request: this request would eventually time out.
    ASSERT_TRUE(not future.is_ready());
}


TEST_F(riak_client_mocked_for_two_requests, client_survives_extra_data_in_put_response)
{
    RpbContent val;
    val.set_value("balooooooga!");
    auto future = client->bucket("a")["document"]->put(val);

    // Respond to the read-before-write GET.
    EXPECT_CALL(*close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    std::string long_reply = clean_put_reply() + "aueoauseonsauenats";
    EXPECT_CALL(*close_request_2, exercise()).Times(1);
    request_handler_2(std::error_code(), long_reply.size(), long_reply);
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}


TEST_F(riak_client_mocked_for_two_requests, client_accepts_well_formed_RbpPutResp)
{
    // Run the actual client.
    RpbContent val;
    val.set_value("balooooooga!");
    auto future = client->bucket("a")["document"]->put(val);

    // Respond to the read-before-write GET.
    EXPECT_CALL(*close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with PUT response.
    EXPECT_CALL(*close_request_2, exercise()).Times(1);
    request_handler_2(std::error_code(), clean_put_reply().size(), clean_put_reply());
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}


TEST_F(riak_client_mocked_for_two_requests, client_accepts_well_formed_put_response_in_parts)
{
    // Run the actual client.
    RpbContent val;
    val.set_value("balooooooga!");
    auto future = client->bucket("a")["document"]->put(val);

    // Respond to the read-before-write GET.
    EXPECT_CALL(*close_request_1, exercise()).Times(1);
    request_handler_1(std::error_code(), clean_fetch_reply().size(), clean_fetch_reply());

    // Proceed with the two-part Put response
    auto data = clean_put_reply();
    assert(data.size() >= 2);
    auto first_half = data.substr(0, data.size() / 2);
    auto second_half = data.substr(data.size() / 2, data.size() - first_half.size());

    // ... part 1
    request_handler_2(std::error_code(), first_half.size(), first_half);
    ASSERT_TRUE(not future.is_ready());

    // ... part 2
    EXPECT_CALL(*close_request_2, exercise()).Times(1);
    request_handler_2(std::error_code(), second_half.size(), second_half);
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
