#pragma once
#include "socket.hxx"

namespace asio = boost::asio;

//=============================================================================
namespace riak {
	namespace transport {
		namespace single_serial_socket {
//=============================================================================

class asio_tcp_socket
	: public socket
{
  public:
  	asio_tcp_socket (asio::io_service& ios)
  	  : implementation_(ios)
  	{   }

  	virtual void cancel () {
  		implementation_.cancel();
  	}

  	virtual void close () {
  		implementation_.close();
  	}

	virtual void shutdown (asio::ip::tcp::socket::shutdown_type type) {
		implementation_.shutdown(type);
	}

	virtual void async_read_some (boost::asio::streambuf& buffer, ReadHandler handler) {
		implementation_.async_read_some(buffer.prepare(1024), handler);
	}

	virtual void async_write_some (const asio::const_buffer& buffer, WriteHandler handler) {
		implementation_.async_write_some(asio::buffer(buffer), handler);
	}

	virtual boost::system::error_code connect (const asio::ip::basic_resolver_entry<asio::ip::tcp>& endpoint, boost::system::error_code& error) {
		return implementation_.connect(endpoint, error);
	}

  private:
	asio::ip::tcp::socket implementation_;
};

//=============================================================================
		}   // namespace single_serial_socket
	}   // namespace transport
}   // namespace riak
//=============================================================================
