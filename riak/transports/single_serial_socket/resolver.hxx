#pragma once
#include <boost/asio/ip/tcp.hpp>

//=============================================================================
namespace riak {
	namespace transport {
		namespace single_serial_socket {
//=============================================================================

class resolver
{
  public:
	virtual ~resolver ()
	{   }

	typedef boost::asio::ip::tcp::resolver::iterator iterator;
	typedef boost::asio::ip::tcp::resolver::query query;

	virtual iterator resolve (const query&) = 0;
};

//=============================================================================
		}   // namespace single_serial_socket
	}   // namespace transport
}   // namespace riak
//=============================================================================
