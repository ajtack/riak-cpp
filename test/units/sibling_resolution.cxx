/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/message.hxx>
#include <test/fixtures/get_with_siblings.hxx>
#include <test/mocks/put_request.hxx>
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
    x->set_vtag("x's tag");
    RpbContent* y = response.add_content();
    y->set_value("y");
    y->set_vtag("y's vtag");
    response.set_vclock("blah blah blah");
    return response;
}

//=============================================================================
        }   // namespace (anonymous)
//=============================================================================

TEST_F(get_with_siblings, getting_siblinged_value_triggers_resolution)
{
    client.get_object("a", "document", response_handler);

    // Handle the GET response.
    auto resolved_sibling = std::make_shared<object>();
    resolved_sibling->set_value("something must be here for serialization!");
    resolved_sibling->set_vtag("blah");
    EXPECT_CALL(sibling_resolution, evaluate(_)).WillOnce(Return(resolved_sibling));

    // Server produces GET response
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());
}


TEST_F(get_with_siblings, null_resolution_result_yields_a_nonerror_result)
{
    client.get_object("a", "document", response_handler);

    // Prepare to handle the GET response, identifying a NULL sibling.
    auto null_result = std::shared_ptr<object>();
    ON_CALL(sibling_resolution, evaluate(_)).WillByDefault(Return(null_result));
    EXPECT_CALL(response_handler_mock, execute(
            Eq(riak::make_error_code()),
            IsNull(),
            _));

    // Server produces siblings
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());
}


TEST_F(get_with_siblings, value_update_after_null_resolution_produces_new_sibling)
{
    client.get_object("a", "document", response_handler);

    // Prepare to handle the GET response.
    auto null_result = std::shared_ptr<object>();
    ON_CALL(sibling_resolution, evaluate(_)).WillByDefault(Return(null_result));
    ::riak::value_updater add_value;
    EXPECT_CALL(response_handler_mock, execute(_, _, _))
        .WillOnce(SaveArg<2>(&add_value));

    // Server produces siblings
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());

    // Prepare to capture a PUT to the server
    std::string second_request_to_server;
    EXPECT_CALL(transport, deliver(_, _))
        .WillOnce(DoAll(
                SaveArg<0>(&second_request_to_server),
                SaveArg<1>(&send_from_server),
                Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))));

    auto val = std::make_shared<object>();
    val->set_value("something new!");

    // Add a new value (PUT now)
    mock::put_request::response_handler put_response_handler;
    add_value(val, std::bind(&mock::put_request::response_handler::execute, &put_response_handler, std::placeholders::_1));

    if (not second_request_to_server.empty()) {
        RpbPutReq put_request;
        if (message::retrieve(put_request, second_request_to_server.size(), second_request_to_server)) {
            ASSERT_TRUE(not put_request.has_vclock());
        } else {
            ADD_FAILURE() << "The value updater did not produce a PUT request to the server; produced something else?";
        }
    } else {
        ADD_FAILURE() << "The value updater did not produce any request to the server.";
    }
}


TEST_F(get_with_siblings, resolved_sibling_is_returned_to_server)
{
    client.get_object("a", "document", response_handler);

    // Handle the GET response.
    auto resolved_sibling = std::make_shared<object>();
    resolved_sibling->CopyFrom(multi_value_get_response().content().Get(0));
    EXPECT_CALL(sibling_resolution, evaluate(_)).WillOnce(Return(resolved_sibling));
    std::string second_request_to_server;
    EXPECT_CALL(transport, deliver(_, _))
            .WillOnce(DoAll(
                    SaveArg<0>(&second_request_to_server),
                    SaveArg<1>(&send_from_server),
                    Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))));

    // Server produces GET response
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());

    // Check that the following PUT contained the expected sibling.
    if (not second_request_to_server.empty()) {
        RpbPutReq put_request;
        if (message::retrieve(put_request, second_request_to_server.size(), second_request_to_server)) {
            ASSERT_EQ(resolved_sibling->value(), put_request.content().value());
            ASSERT_TRUE(put_request.has_vclock());
            ASSERT_EQ(multi_value_get_response().vclock(), put_request.vclock());
            ASSERT_EQ(resolved_sibling->vtag(), put_request.content().vtag());
        } else {
            ADD_FAILURE() << "Sibling resolution produced something other than a PUT request to the server!";
        }
    } else {
        ADD_FAILURE() << "Sibling resolution produced an empty PUT request to the server!";
    }
}


TEST_F(get_with_siblings, resolved_sibling_produces_get_result)
{
    client.get_object("a", "document", response_handler);

    // Handle the GET response.
    auto resolved_sibling = std::make_shared<object>();
    resolved_sibling->CopyFrom(multi_value_get_response().content().Get(0));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(AnyNumber()).WillRepeatedly(Return(resolved_sibling));
    std::string second_request_to_server;
    ON_CALL(transport, deliver(_, _))
            .WillByDefault(DoAll(
                    SaveArg<0>(&second_request_to_server),
                    SaveArg<1>(&send_from_server),
                    Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))));

    // Server produces GET response
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());

    // Check that the following PUT contained the expected sibling.
    if (not second_request_to_server.empty()) {
        RpbPutReq put_request;
        if (message::retrieve(put_request, second_request_to_server.size(), second_request_to_server)) {
            // Respond to PUT with the same sibling
            EXPECT_CALL(response_handler_mock, execute(
                    Eq(riak::make_error_code()),
                    Pointee(Property(&riak::object::value, StrEq(resolved_sibling->value()))),
                    _));
            RpbPutResp put_response;
            put_response.set_vclock(put_request.vclock());
            if (put_request.return_body()) {
                RpbContent* response_content = put_response.add_content();
                response_content->set_value(put_request.content().value());
            } else if (put_request.return_head()) {
                RpbContent* response_content = put_response.add_content();
                response_content->set_value("");
            }

            std::string encoded_response;
            put_response.SerializeToString(&encoded_response);
            riak::message::wire_package wire_response(riak::message::code::PutResponse, encoded_response);
            send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());
        } else {
            ADD_FAILURE() << "Sibling resolution produced something other than a PUT request to the server!";
        }
    } else {
        ADD_FAILURE() << "Sibling resolution produced an empty PUT request to the server!";
    }
}


TEST_F(get_with_siblings, resolving_sibling_handles_erroneous_server_reply)
{
    client.get_object("a", "document", response_handler);

    // Prepare for the get response, which in this case causes sibling resolution.
    auto resolved_sibling = std::make_shared<object>();
    resolved_sibling->CopyFrom(multi_value_get_response().content().Get(0));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(AnyNumber()).WillRepeatedly(Return(resolved_sibling));
    std::string second_request_to_server;
    ON_CALL(transport, deliver(_, _))
            .WillByDefault(DoAll(
                    SaveArg<0>(&second_request_to_server),
                    SaveArg<1>(&send_from_server),
                    Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))));

    // Server produces GET response (with siblings), triggering resolution and a new PUT response.
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());

    // Now the server misbehaves with the PUT response.
    if (not second_request_to_server.empty()) {
        RpbPutReq put_request;
        if (message::retrieve(put_request, second_request_to_server.size(), second_request_to_server)) {
            EXPECT_CALL(response_handler_mock, execute(
                    Ne(riak::make_error_code()),
                    _,
                    _));
            RpbPutResp put_response;
            std::string encoded_response;
            put_response.SerializeToString(&encoded_response);   // Notice: wrong code!
            riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
            send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());
        } else {
            ADD_FAILURE() << "Sibling resolution produced something other than a PUT request to the server!";
        }
    } else {
        ADD_FAILURE() << "Sibling resolution produced an empty PUT request to the server!";
    }
}


TEST_F(get_with_siblings, resolving_sibling_handles_server_failure)
{
    client.get_object("a", "document", response_handler);

    // Prepare for the get response, which in this case causes sibling resolution.
    auto resolved_sibling = std::make_shared<object>();
    resolved_sibling->CopyFrom(multi_value_get_response().content().Get(0));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(AnyNumber()).WillRepeatedly(Return(resolved_sibling));
    std::string second_request_to_server;
    ON_CALL(transport, deliver(_, _))
            .WillByDefault(DoAll(
                    SaveArg<0>(&second_request_to_server),
                    SaveArg<1>(&send_from_server),
                    Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))));

    // Server produces GET response (with siblings), triggering resolution and a new PUT response.
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package wire_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());

    // Now the server misbehaves with the PUT response.
    if (not second_request_to_server.empty()) {
        RpbPutReq put_request;
        if (message::retrieve(put_request, second_request_to_server.size(), second_request_to_server)) {
            EXPECT_CALL(response_handler_mock, execute(Eq(std::errc::connection_aborted), _, _));
            send_from_server(std::make_error_code(std::errc::connection_aborted), 0, "");
        } else {
            ADD_FAILURE() << "Sibling resolution produced something other than a PUT request to the server!";
        }
    } else {
        ADD_FAILURE() << "Sibling resolution produced an empty PUT request to the server!";
    }
}


TEST_F(get_with_siblings, multiple_sibling_resolutions_are_correctly_handled)
{
    client.get_object("a", "document", response_handler);

    // Handle the GET response.
    auto first_resolved_sibling = std::make_shared<object>();
    first_resolved_sibling->CopyFrom(multi_value_get_response().content().Get(0));
    EXPECT_CALL(sibling_resolution, evaluate(_)).Times(AnyNumber()).WillRepeatedly(Return(first_resolved_sibling));
    std::string second_request_to_server;
    ON_CALL(transport, deliver(_, _))
            .WillByDefault(DoAll(
                    SaveArg<0>(&second_request_to_server),
                    SaveArg<1>(&send_from_server),
                    Return(std::bind(&mock::transport::device::option_to_terminate_request::exercise, &closure_signal))));

    // Server produces GET response, triggering the above sibling resolution.
    std::string encoded_response;
    multi_value_get_response().SerializeToString(&encoded_response);
    riak::message::wire_package multisibling_response(riak::message::code::GetResponse, encoded_response);
    send_from_server(std::error_code(), multisibling_response.to_string().size(), multisibling_response.to_string());

    // Respond to the resolved sibling with more siblings, causing another PUT requset
    if (not second_request_to_server.empty()) {
        RpbPutReq put_request;
        if (message::retrieve(put_request, second_request_to_server.size(), second_request_to_server)) {
            // Resolve siblings...
            auto second_resolved_sibling = std::make_shared<object>();
            second_resolved_sibling->CopyFrom(multi_value_get_response().content().Get(1));
            EXPECT_CALL(sibling_resolution, evaluate(_))
                    .Times(AnyNumber())
                    .WillRepeatedly(Return(second_resolved_sibling));

            // Expect a third request: fetching the bodies of the siblings.
            std::string third_request_to_server;
            EXPECT_CALL(transport, deliver(_, _))
                    .WillOnce(DoAll(
                            SaveArg<0>(&third_request_to_server),
                            SaveArg<1>(&send_from_server),
                            Return(std::bind(
                                    &mock::transport::device::option_to_terminate_request::exercise, &closure_signal))));
            
            // Produce the response to request #2: multi-sibling put response.
            RpbPutResp put_response;
            put_response.set_vclock(put_request.vclock());
            RpbContent* response_content_1 = put_response.add_content();
            response_content_1->set_value("");
            RpbContent* response_content_2 = put_response.add_content();
            response_content_2->set_value("");

            std::string encoded_response;
            put_response.SerializeToString(&encoded_response);
            riak::message::wire_package wire_response(riak::message::code::PutResponse, encoded_response);
            send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());

            if (not third_request_to_server.empty()) {
                RpbGetReq second_get_request;   // A PUT would have returned only heads, so another GET is needed.
                if (message::retrieve(second_get_request, third_request_to_server.size(), third_request_to_server)) {
                    // Expect a fourth request: the second PUT resolution
                    std::string fourth_request_to_server;
                    EXPECT_CALL(transport, deliver(_, _))
                            .WillOnce(DoAll(
                                    SaveArg<0>(&fourth_request_to_server),
                                    SaveArg<1>(&send_from_server),
                                    Return(std::bind(
                                            &mock::transport::device::option_to_terminate_request::exercise,
                                            &closure_signal))));

                    // Send the same siblings again; hope this is fine.
                    send_from_server(
                            std::error_code(),
                            multisibling_response.to_string().size(),
                            multisibling_response.to_string());
                    
                    if (not fourth_request_to_server.empty()) {
                        RpbPutReq second_put_request;
                        if (message::retrieve(second_put_request, fourth_request_to_server.size(), fourth_request_to_server)) {
                            // Respond to the last PUT with the same sibling, resulting in a successful GET.
                            EXPECT_CALL(response_handler_mock, execute(
                                    Eq(riak::make_error_code()),
                                    Pointee(Property(&riak::object::value, StrEq(second_resolved_sibling->value()))),
                                    _));
                            RpbPutResp put_response;
                            put_response.set_vclock(put_request.vclock());
                            if (put_request.return_body()) {
                                RpbContent* response_content = put_response.add_content();
                                response_content->set_value(put_request.content().value());  // value doesn't matter.
                            } else if (put_request.return_head()) {
                                RpbContent* response_content = put_response.add_content();
                                response_content->set_value("");
                            }

                            std::string encoded_response;
                            put_response.SerializeToString(&encoded_response);
                            riak::message::wire_package wire_response(riak::message::code::PutResponse, encoded_response);
                            send_from_server(std::error_code(), wire_response.to_string().size(), wire_response.to_string());
                        } else {
                            ADD_FAILURE() << "Second-round sibling resolution produced something other than a PUT!";                    
                        }
                    } else {
                        ADD_FAILURE() << "Second-round sibling resolution produced an empty request to the server!";
                    }
                } else {
                    ADD_FAILURE() << "Expected a second-round GET, received something else!";                    
                }
            } else {
                ADD_FAILURE() << "Expected a second-round sibling resolution GET request!";
            }
        } else {
            ADD_FAILURE() << "Sibling resolution produced something other than a PUT request to the server!";
        }
    } else {
        ADD_FAILURE() << "Sibling resolution produced an empty request to the server!";
    }

    // There has GOT to be a better way to write this test.
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
