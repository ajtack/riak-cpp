#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/store.hxx>
#include <test/mocks/transport.hxx>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

struct riak_store_mocked_for_two_requests
       : public ::testing::Test 
{
    riak_store_mocked_for_two_requests ();
    ~riak_store_mocked_for_two_requests ();

    mock::transport transport;
    boost::asio::io_service ios;
    riak::store store;
    transport::response_handler request_handler_1;
    transport::response_handler request_handler_2;
    std::shared_ptr<mock::transport::option_to_terminate_request> close_request_1;
    std::shared_ptr<mock::transport::option_to_terminate_request> close_request_2;
};

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
