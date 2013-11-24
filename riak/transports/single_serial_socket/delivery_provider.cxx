#include "asio_tcp_socket.hxx"
#include "asio_tcp_resolver.hxx"
#include "delivery_provider.hxx"
#include "scheduler.hxx"

//=============================================================================
namespace riak {
	namespace transport {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;

transport::delivery_provider make_single_socket_transport (
        const std::string& address,
        uint16_t port,
        boost::asio::io_service& ios)
{
	std::unique_ptr<single_serial_socket::socket> socket(new single_serial_socket::asio_tcp_socket(ios));
	std::shared_ptr<single_serial_socket::resolver> resolver(new single_serial_socket::asio_tcp_resolver(ios));

    auto transport = std::make_shared<single_serial_socket::scheduler>(address, port, ios, std::move(socket), resolver);
    return std::bind(&single_serial_socket::scheduler::deliver, transport, _1, _2);
}

//=============================================================================
	}   // 	namespace transport
}   // namespace riak
//=============================================================================
