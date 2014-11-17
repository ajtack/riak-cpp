#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/client.hxx>
#include <test/fixtures/log/logs_test_name.hxx>
#include <test/mocks/transport.hxx>

namespace riak {
    namespace mock {
        namespace utility { class timer_factory; }
    }
}

//=============================================================================
namespace riak {
    namespace test {
        namespace fixture {
//=============================================================================

struct riak_client_mocked_for_two_requests
       : public logs_test_name
{
    riak_client_mocked_for_two_requests ();
    ~riak_client_mocked_for_two_requests ();

    mock::transport::device transport;
    std::shared_ptr<mock::utility::timer_factory> timer_factory_mock;
    riak::client client;
    std::string received_request_1;
    std::string received_request_2;
    transport::response_handler request_handler_1;
    transport::response_handler request_handler_2;
    mock::transport::device::option_to_terminate_request close_request_1;
    mock::transport::device::option_to_terminate_request close_request_2;
};

//=============================================================================
        }   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
