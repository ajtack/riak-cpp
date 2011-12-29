#pragma once
#include <test/fixtures/riak-client-with-mocked-transport.hxx>
#include <test/mocks/delete_request.hxx>

//=============================================================================
namespace riak {
    namespace test {
    	namespace fixture {
//=============================================================================

struct deleting_client
       : public riak_client_with_mocked_transport
{
    typedef mock::delete_request::response_handler mock_response_handler;

    deleting_client ();

    mock_response_handler response_handler_mock;
    ::riak::delete_response_handler response_handler;
};

//=============================================================================
		}   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
