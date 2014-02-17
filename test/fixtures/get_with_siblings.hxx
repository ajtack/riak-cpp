#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/client.hxx>
#include <test/fixtures/logs_test_name.hxx>
#include <test/mocks/get_request.hxx>
#include <test/mocks/transport.hxx>
#include <test/mocks/sibling_resolution.hxx>

//=============================================================================
namespace riak {
    namespace test {
        namespace fixture {
//=============================================================================

struct get_with_siblings
       : public logs_test_name
{
    get_with_siblings ();
    ~get_with_siblings ();

    typedef mock::get_request::response_handler mock_response_handler;
    
    testing::NiceMock<mock::transport::device> transport;
    testing::NiceMock<mock::sibling_resolution> sibling_resolution;
    boost::asio::io_service ios;
    riak::client client;
    mock_response_handler response_handler_mock;
    ::riak::get_response_handler response_handler;
    
    ::riak::transport::response_handler send_from_server;
    testing::NiceMock<mock::transport::device::option_to_terminate_request> closure_signal;
};

//=============================================================================
        }   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
