#pragma once
#include <test/mocks/transport/single_serial_socket/socket.hxx>
#include <riak/transports/single_serial_socket/scheduler.hxx>
#include <boost/asio/io_service.hpp>

namespace riak {
	namespace mock { namespace sss = transport::single_serial_socket; }
}

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

struct single_serial_socket_transport_with_working_connection
       : public ::testing::Test
{
	single_serial_socket_transport_with_working_connection ();
	virtual ~single_serial_socket_transport_with_working_connection ();
	
	/*!
	 * Useful for injecting errors or responses in a transport workflow via the
	 * async_(read|write)_some functions.
	 */
	::testing::NiceMock<mock::sss::socket>* const socket;

	/*! Connected using socket, will always be able to re-connect in case of a connection drop. */
	std::unique_ptr<riak::transport::single_serial_socket::scheduler> transport;

	boost::asio::io_service ios;
};

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
