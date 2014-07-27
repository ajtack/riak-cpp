#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/client.hxx>
#include <test/fixtures/log/logs_test_name.hxx>
#include <test/mocks/get_request.hxx>
#include <test/mocks/transport.hxx>
#include <test/mocks/sibling_resolution.hxx>

//=============================================================================
namespace riak {
    namespace test {
        namespace fixture {
//=============================================================================

struct riak_client_with_mocked_transport
       : public logs_test_name 
{
    riak_client_with_mocked_transport ();
    ~riak_client_with_mocked_transport ();

    typedef mock::get_request::response_handler mock_response_handler;

    mock::transport::device transport;
    mock::sibling_resolution sibling_resolution;
    boost::asio::io_service ios;
    riak::client client;
    mock_response_handler response_handler_mock;
    ::riak::get_response_handler response_handler;

    ::riak::transport::response_handler send_from_server;
    mock::transport::device::option_to_terminate_request closure_signal;
};

//=============================================================================
        }   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
