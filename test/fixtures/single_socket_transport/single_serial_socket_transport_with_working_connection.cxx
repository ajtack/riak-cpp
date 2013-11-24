#include "single_serial_socket_transport_with_working_connection.hxx"
#include <test/mocks/transport/single_serial_socket/resolver.hxx>
#include <test/mocks/transport/single_serial_socket/socket.hxx>
#include <gmock/gmock.h>

using namespace ::testing;
namespace single_serial_socket = ::riak::transport::single_serial_socket;
namespace riak {
	namespace mock { namespace sss = transport::single_serial_socket; }
}

//=============================================================================
namespace riak {
    namespace test {
        namespace fixture {
//=============================================================================

single_serial_socket_transport_with_working_connection::single_serial_socket_transport_with_working_connection ()
  : socket(new NiceMock<mock::sss::socket>)
{
	// Resolve the target address to something -- we don't care.
	auto resolver = std::make_shared<NiceMock<mock::sss::resolver>>();
	auto dns_result = mock::sss::resolver::iterator::create(boost::asio::ip::tcp::endpoint(), "boo", "bear");
	ON_CALL(*resolver, resolve(_)).WillByDefault(Return(dns_result));

	// Succeed in connecting every time.
	auto success = boost::system::error_code();
	ON_CALL(*socket, connect(_, _)).WillByDefault(SetArgReferee<1>(success));

	// Remember the physical pointer to this socket -- the pool must own the canonical (unique_ptr) pointer.
	std::unique_ptr<NiceMock<mock::sss::socket>> socket_ptr(socket);
	
	transport.reset(
			new single_serial_socket::scheduler(
					"wherever", 8000, ios,
					std::move(socket_ptr),
					std::static_pointer_cast<single_serial_socket::resolver>(resolver))
		);
}


single_serial_socket_transport_with_working_connection::~single_serial_socket_transport_with_working_connection ()
{   }

//=============================================================================
        }   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
