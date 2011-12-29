#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/client.hxx>
#include <test/mocks/transport.hxx>
#include <test/mocks/sibling_resolution.hxx>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

struct riak_client_with_mocked_transport
       : public ::testing::Test 
{
    riak_client_with_mocked_transport ();
    ~riak_client_with_mocked_transport ();

    mock::transport transport;
    mock::sibling_resolution sibling_resolution;
    boost::asio::io_service ios;
    std::shared_ptr<riak::client> client;
    transport::response_handler request_handler;
    mock::transport::option_to_terminate_request closure_signal;
};

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
