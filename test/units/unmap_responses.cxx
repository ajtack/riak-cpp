/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/units/riak-store-with-mocked-transport.hxx>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

TEST_F(riak_store_with_mocked_transport, store_survives_nonsense_reply_to_unmap)
{
    auto future = store["a"].unmap("document");
    
    EXPECT_CALL(*closure_signal, exercise());
    std::string garbage("uhetnaoutaenosueosaueoas");
    request_handler(std::error_code(), garbage.size(), garbage);
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_exception());
    ASSERT_THROW(future.get(), std::invalid_argument);
}


TEST_F(riak_store_with_mocked_transport, store_survives_extra_RpbDelResp_from_unmap)
{
    auto future = store["a"].unmap("document");

    EXPECT_CALL(*closure_signal, exercise());
    riak::message::wire_package long_reply(riak::message::code::DeleteResponse, "atnhueoauheas(garbage)");
    request_handler(std::error_code(), long_reply.to_string().size(), long_reply.to_string());
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}


TEST_F(riak_store_with_mocked_transport, store_accepts_well_formed_RbpDelResp)
{
    auto future = store["a"].unmap("document");
    
    EXPECT_CALL(*closure_signal, exercise());
    riak::message::wire_package clean_reply(riak::message::code::DeleteResponse, "");
    request_handler(std::error_code(), clean_reply.to_string().size(), clean_reply.to_string());
    ASSERT_TRUE(future.is_ready());
    ASSERT_TRUE(future.has_value());
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
