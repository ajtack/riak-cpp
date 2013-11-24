#pragma once
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>

//=============================================================================
namespace riak {
	namespace transport {
		namespace single_serial_socket {
//=============================================================================

/*!
 * An interface to the features used by the socket implementation, for  the primary purpose of
 * unit testing.
 */
class socket
{
  public:
	virtual ~socket ()
	{   }

	typedef std::function<void(const boost::system::error_code&, std::size_t)> ReadHandler;
	typedef std::function<void(const boost::system::error_code&, std::size_t)> WriteHandler;

	virtual void cancel () = 0;
	virtual void close () = 0;
	virtual void shutdown (boost::asio::ip::tcp::socket::shutdown_type) = 0;
	virtual void async_read_some (boost::asio::streambuf&, ReadHandler) = 0;
	virtual void async_write_some (const boost::asio::const_buffer&, WriteHandler) = 0;
	virtual boost::system::error_code connect (const boost::asio::ip::basic_resolver_entry<boost::asio::ip::tcp>&, boost::system::error_code&) = 0;
};

//=============================================================================
		}   // namespace single_serial_socket
	}   // namespace transport
}   // namespace riak
//=============================================================================
