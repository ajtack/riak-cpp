#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/store.hxx>
#include <test/mocks/transport.hxx>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

struct riak_store_with_mocked_transport
       : public ::testing::Test 
{
    riak_store_with_mocked_transport ();

    mock::transport transport;
    boost::asio::io_service ios;
    riak::store store;
    std::shared_ptr<mock::transport::option_to_terminate_request> closure_signal;
};

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
