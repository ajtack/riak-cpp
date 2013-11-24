#pragma once
#include <riak/transport.hxx>
#include <gmock/gmock.h>

//=============================================================================
namespace riak {
    namespace mock {
        namespace transport {
//=============================================================================

class device
{
public:
    class option_to_terminate_request;
    class response_handler;

    MOCK_METHOD2( deliver,
            ::riak::transport::option_to_terminate_request(
                    const std::string&,
                    ::riak::transport::response_handler) );
};

class device::option_to_terminate_request
{
public:
    MOCK_METHOD0( exercise, void() );
};

class device::response_handler
{
  public:
	MOCK_METHOD3(execute, void(const std::error_code&, std::size_t, std::string));
};

//=============================================================================
        }   // namespace transport
    }   // namespace mock
}   // namespace riak
//=============================================================================
