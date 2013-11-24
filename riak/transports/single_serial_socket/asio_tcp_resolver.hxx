#include "resolver.hxx"
#include <boost/asio/ip/basic_resolver.hpp>

//=============================================================================
namespace riak {
	namespace transport {
		namespace single_serial_socket {
//=============================================================================

class asio_tcp_resolver
	: public single_serial_socket::resolver
{
  public:
	asio_tcp_resolver(asio::io_service& ios)
	  : impl_(ios)
	{   }

	iterator resolve (const query& q) {
		return impl_.resolve(q);
	}

  private:
	boost::asio::ip::tcp::resolver impl_;
};

//=============================================================================
		}   // namespace single_serial_socket
	}   // namespace transport
}   // namespace riak
//=============================================================================
