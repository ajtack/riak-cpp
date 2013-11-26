/*!
 * \file
 * Implements unit tests for the treatment of transport subclasses by the Riak client. A
 * programmer implementing his own connection pool may look to this as the specification of how
 * his pool will be used.
 */
#include <gtest/gtest.h>
#include <riak/transports/single_serial_socket/scheduler.hxx>
#include <test/fixtures/single_socket_transport/single_serial_socket_transport_with_working_connection.hxx>
#include <test/mocks/transport.hxx>
#include <ostream>
#include <boost/bind.hpp>

using namespace ::testing;
using riak::test::fixture::single_serial_socket_transport_with_working_connection;
namespace single_serial_socket = ::riak::transport::single_serial_socket;
namespace riak {
	namespace mock { namespace sss = transport::single_serial_socket; }
}

//=============================================================================
namespace riak {
	namespace test {
		namespace {
//=============================================================================

// in the shape of async_write_some
class ReportFullBufferWritten
{
  public:
	ReportFullBufferWritten(boost::asio::io_service& ios)
	  : ios_(ios)
	{   }

	void operator() (const boost::asio::const_buffer& b, transport::single_serial_socket::socket::WriteHandler h) {
		// std::bind yielded a stack overflow here.
		std::function<void()> f = boost::bind(h, boost::system::error_code(), boost::asio::buffer_size(b));
		ios_.post(f);
	}

  private:
	boost::asio::io_service& ios_;
};

// in the shape of async_read_some
class DeliverResponseMessage
{
  public:
	DeliverResponseMessage (const std::string& content, boost::asio::io_service& ios)
	  : content_(content)
	  , ios_(ios)
	{   }

	void operator() (boost::asio::streambuf& b, transport::single_serial_socket::socket::ReadHandler h) {
		std::ostream stream(&b);
		stream << content_;

		// std::bind yielded a stack overflow here.
		std::function<void()> f = boost::bind(h, boost::system::error_code(), content_.length());
		ios_.post(f);
	}

  private:
  	std::string content_;
	boost::asio::io_service& ios_;
};

// in the shape of async_read_some
template <typename BufferType>
class YieldError
{
  public:
	YieldError (boost::system::error_code e, boost::asio::io_service& ios)
	  : error_(e)
	  , ios_(ios)
	{   }

	void operator() (BufferType& b, transport::single_serial_socket::socket::ReadHandler h) {
		// std::bind yielded a stack overflow here.
		std::function<void()> f = boost::bind(h, error_, 0);
		ios_.post(f);
	}

  private:
	boost::system::error_code error_;
	boost::asio::io_service& ios_;
};

typedef YieldError<boost::asio::streambuf> YieldErrorOnRead;
typedef YieldError<const boost::asio::const_buffer> YieldErrorOnWrite;

// in the shape of transport::response_handler
class InvokeTerminateOption
{
  public:
	InvokeTerminateOption (transport::option_to_terminate_request terminate, bool end_cleanly = true)
	  : terminate_(terminate)
	  , ended_cleanly_(end_cleanly)
	{   }

	void operator () (std::error_code, std::size_t, const std::string&) {
		terminate_(!ended_cleanly_);
	}

  private:
	transport::option_to_terminate_request terminate_;
	bool ended_cleanly_;
};

//=============================================================================
		}   // namespace (anonymous)
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

TEST_F(single_serial_socket_transport_with_working_connection, complete_response_delivered_correctly)
{
	// Return the response as anticipated.
	const std::string expected_response = "cheesy brains!";
	ON_CALL(*socket, async_write_some(_, _))
		.WillByDefault( Invoke(ReportFullBufferWritten(ios)) );
	ON_CALL(*socket, async_read_some(_, _))
		.WillByDefault( Invoke(DeliverResponseMessage(expected_response, ios)) );

	// The meat of the test!
	mock::transport::device::response_handler handler;
	auto t = transport->deliver("what do mouse zombies like to eat?", std::bind(&mock::transport::device::response_handler::execute, &handler, _1, _2, _3));
	auto error_code = std::make_error_code(static_cast<std::errc>(boost::system::error_code().value()));
	EXPECT_CALL(handler, execute(error_code, expected_response.size(), Eq(expected_response)))
		.WillOnce(Invoke(InvokeTerminateOption(t)));

	ios.run();
}


TEST_F(single_serial_socket_transport_with_working_connection, connection_failure_errors_reported_while_reading)
{
	// Write successfully.
	ON_CALL(*socket, async_write_some(_, _))
		.WillByDefault( Invoke(ReportFullBufferWritten(ios)) );

	// Return an error while reading the response.
	ON_CALL(*socket, async_read_some(_, _))
		.WillByDefault( Invoke(YieldErrorOnRead(boost::asio::error::connection_reset, ios)) );

	// Make sure we get this error at the handler.
	mock::transport::device::response_handler handler;
	auto t = transport->deliver("what do mouse zombies like to eat?", std::bind(&mock::transport::device::response_handler::execute, &handler, _1, _2, _3));
	auto error_code = std::make_error_code(static_cast<std::errc>(boost::asio::error::connection_reset));
	EXPECT_CALL(handler, execute(error_code, 0, "")).WillOnce(Invoke(InvokeTerminateOption(t)));

	ios.run();
}


TEST_F(single_serial_socket_transport_with_working_connection, connection_failure_errors_reported_while_writing)
{
	// Fail to write to the socket.
	ON_CALL(*socket, async_write_some(_, _))
		.WillByDefault( Invoke(YieldErrorOnWrite(boost::asio::error::connection_reset, ios)) );

	// Make sure we get this error at the handler.
	mock::transport::device::response_handler handler;
	auto t = transport->deliver("what do mouse zombies like to eat?", std::bind(&mock::transport::device::response_handler::execute, &handler, _1, _2, _3));
	auto error_code = std::make_error_code(static_cast<std::errc>(boost::asio::error::connection_reset));
	EXPECT_CALL(handler, execute(error_code, 0, "")).WillOnce(Invoke(InvokeTerminateOption(t)));

	ios.run();
}

//=============================================================================
	}   // namespace test
}   // namespace riak
//=============================================================================
