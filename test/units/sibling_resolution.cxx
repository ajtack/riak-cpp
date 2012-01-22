/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/fixtures/get_with_siblings.hxx>
#include <system_error>

using namespace ::testing;
using riak::test::fixture::get_with_siblings;

//=============================================================================
namespace riak {
    namespace test {
        namespace {
//=============================================================================

RpbGetResp multi_value_get_response ()
{
    RpbGetResp response;
    RpbContent* x = response.add_content();
    x->set_value("x");
    RpbContent* y = response.add_content();
    y->set_value("y");
    response.set_vclock("blah blah blah");
    return response;
}

//=============================================================================
        }   // namespace (anonymous)
//=============================================================================

TEST_F(get_with_siblings, getting_siblinged_value_triggers_resolution)
{
    client->get_object("a", "document", response_handler);

    // Handle the GET response.
    EXPECT_CALL(sibling_resolution, evaluate(_)).WillOnce(Return(0));

    // Server produces GET response
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());
}


TEST_F(get_with_siblings, resolved_sibling_is_returned_to_server)
{
    client->get_object("a", "document", response_handler);

    // Handle the GET response.
    auto resolved_sibling_value = multi_value_get_response().content().Get(0).value();
    EXPECT_CALL(sibling_resolution, evaluate(_)).WillOnce(Return(0));
    std::string second_request_to_server;
    EXPECT_CALL(transport, deliver(_, _))
            .WillOnce(DoAll(
                    SaveArg<0>(&second_request_to_server),
                    SaveArg<1>(&send_from_server),
                    Return(std::bind(&mock::transport::option_to_terminate_request::exercise, &closure_signal))));

    // Server produces GET response
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());

    // Check that the following PUT contained the expected sibling.
    if (not second_request_to_server.empty()) {
        RpbPutReq put_request;
        if (message::retrieve(put_request, second_request_to_server.size(), second_request_to_server)) {
            ASSERT_EQ(resolved_sibling_value, put_request.content().value());
        } else {
            ADD_FAILURE() << "Sibling resolution produced something other than a PUT request to the server!";
        }
    } else {
        ADD_FAILURE() << "Sibling resolution produced an empty PUT request to the server!";
    }
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
