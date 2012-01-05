#pragma once
#include <test/fixtures/riak-client-with-mocked-transport.hxx>
#include <test/mocks/get_request.hxx>

//=============================================================================
namespace riak {
    namespace test {
    	namespace fixture {
//=============================================================================

struct getting_client
       : public riak_client_with_mocked_transport
{
    typedef mock::get_request::response_handler mock_response_handler;

    getting_client ();
    ~getting_client ();

    mock_response_handler response_handler_mock;
    ::riak::get_response_handler response_handler;
};

//=============================================================================
		}   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
