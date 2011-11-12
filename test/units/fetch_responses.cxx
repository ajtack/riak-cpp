/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/units/riak-store-with-mocked-transport.hxx>
#include <system_error>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

auto canned_fetch_response = RpbGetResp();

TEST_F(riak_store_with_mocked_transport, store_survives_long_nonsense_reply_to_fetch)
{
    auto future = store["a"]["document"]->fetch();
    
    EXPECT_CALL(*closure_signal, exercise()).Times(0);
    std::string garbage("uhetnaoutaenosueosaueoas");
    request_handler(std::error_code(), garbage.size(), garbage);
    
    // The above in binary would encode a much longer request: this request would eventually time out.
    ASSERT_TRUE(not future.is_ready());
}


TEST_F(riak_store_with_mocked_transport, store_survives_extra_data_in_fetch_response)
{
    auto future = store["a"]["document"]->fetch();

    EXPECT_CALL(*closure_signal, exercise());
    std::string response_with_extra;
    canned_fetch_response.SerializeToString(&response_with_extra);
    riak::message::wire_package correct_reply(riak::message::code::GetResponse, response_with_extra);
    std::string long_reply = correct_reply.to_string() + "aueoauseonsauenats";
    request_handler(std::error_code(), long_reply.size(), long_reply);
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}


TEST_F(riak_store_with_mocked_transport, store_accepts_well_formed_RbpGetResp)
{
    auto future = store["a"]["document"]->fetch();
    
    EXPECT_CALL(*closure_signal, exercise());
    std::string response_data;
    canned_fetch_response.SerializeToString(&response_data);
    riak::message::wire_package clean_reply(riak::message::code::GetResponse, response_data);
    request_handler(std::error_code(), clean_reply.to_string().size(), clean_reply.to_string());
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}


TEST_F(riak_store_with_mocked_transport, store_accepts_well_formed_response_in_parts)
{
    auto future = store["a"]["document"]->fetch();
    
    std::string response_data;
    canned_fetch_response.SerializeToString(&response_data);
    riak::message::wire_package clean_reply(riak::message::code::GetResponse, response_data);
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
