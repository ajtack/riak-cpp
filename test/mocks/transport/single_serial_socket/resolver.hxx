#pragma once
#include <riak/transports/single_serial_socket/resolver.hxx>
#include <gmock/gmock.h>

//=============================================================================
namespace riak {
    namespace mock {
		namespace transport {
			namespace single_serial_socket {
//=============================================================================

class resolver
    : public ::riak::transport::single_serial_socket::resolver
{
  public:
  	resolver ();
  	virtual ~resolver ();

  	MOCK_METHOD1(resolve, iterator(const query&));
};

//=============================================================================
			}   // namespace single_serial_socket
		}   // namespace transport
	}   // namespace mock
}   // namespace riak
//=============================================================================
