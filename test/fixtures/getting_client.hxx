#pragma once
#include <test/fixtures/riak-client-with-mocked-transport.hxx>
#include <test/fixtures/log/captures_log_output.hxx>
#include <test/mocks/get_request.hxx>

//=============================================================================
namespace riak {
    namespace test {
    	namespace fixture {
//=============================================================================

struct getting_client
       : public riak_client_with_mocked_transport
       , public captures_log_output
{
    typedef mock::get_request::response_handler mock_response_handler;

    getting_client ();
    ~getting_client ();
};

//=============================================================================
		}   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
