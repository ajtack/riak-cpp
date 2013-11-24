#pragma once
#include <riak/transports/single_serial_socket/socket.hxx>
#include <gmock/gmock.h>

//=============================================================================
namespace riak {
    namespace mock {
		namespace transport {
			namespace single_serial_socket {
//=============================================================================

class socket
    : public ::riak::transport::single_serial_socket::socket
{
  public:
	socket ();
	virtual ~socket ();

	MOCK_METHOD0(cancel, void());
	MOCK_METHOD0(close, void());
	MOCK_METHOD1(shutdown, void(boost::asio::ip::tcp::socket::shutdown_type));
	MOCK_METHOD2(async_read_some, void(boost::asio::streambuf&, ReadHandler));
	MOCK_METHOD2(async_write_some, void(const boost::asio::const_buffer&, WriteHandler));
	MOCK_METHOD2(connect, boost::system::error_code(const boost::asio::ip::basic_resolver_entry<boost::asio::ip::tcp>&, boost::system::error_code&));
};

//=============================================================================
			}   // namespace single_serial_socket
		}   // namespace transport
	}   // namespace mock
}   // namespace riak
//=============================================================================
