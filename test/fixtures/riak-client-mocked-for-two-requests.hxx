#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/client.hxx>
#include <test/mocks/transport.hxx>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

struct riak_client_mocked_for_two_requests
       : public ::testing::Test 
{
    riak_client_mocked_for_two_requests ();
    ~riak_client_mocked_for_two_requests ();

    mock::transport::device transport;
    boost::asio::io_service ios;
    std::shared_ptr<riak::client> client;
    std::string received_request_1;
    std::string received_request_2;
    transport::response_handler request_handler_1;
    transport::response_handler request_handler_2;
    mock::transport::device::option_to_terminate_request close_request_1;
    mock::transport::device::option_to_terminate_request close_request_2;
};

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
