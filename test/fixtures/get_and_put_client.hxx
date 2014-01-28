#pragma once
#include <test/fixtures/log/captures_log_output.hxx>
#include <test/fixtures/riak-client-mocked-for-two-requests.hxx>
#include <test/mocks/get_request.hxx>
#include <test/mocks/put_request.hxx>

//=============================================================================
namespace riak {
    namespace test {
        namespace fixture {
//=============================================================================

struct get_and_put_client
       : public riak_client_mocked_for_two_requests
       , captures_log_output
{
    get_and_put_client ();
    ~get_and_put_client ();

    mock::get_request::response_handler get_response_handler_mock;
    ::riak::get_response_handler get_response_handler;

    mock::put_request::response_handler put_response_handler_mock;
    ::riak::put_response_handler put_response_handler;
};

//=============================================================================
        }   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
