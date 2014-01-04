#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/application_request_context.hxx>
#include <test/fixtures/logs_test_name.hxx>
#include <test/mocks/boost/log_sink.hxx>
#include <test/mocks/transport.hxx>

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

class application_request_context
	  : public logs_test_name
{
  public:
	application_request_context ();
	~application_request_context ();

	/*!
	 * On each call, produces a new context sharing a common set of parameters among all
	 * such calls.
	 */
	riak::application_request_context new_context ();

  private:
	riak::object_access_parameters oap_;
	riak::request_failure_parameters rfp_;
	::testing::StrictMock<mock::transport::device> transport_;
	boost::asio::io_service ios_;

	boost::shared_ptr<::testing::NiceMock<mock::boost::log_sink>> log_sinks_;

  public:
	/*!
	 * A representative of all log sinks attached to the boost logging core, with no filtering
	 * in place. Useful for capturing logging output from a request context instance.
	 */
	::testing::NiceMock<mock::boost::log_sink>& log_sinks;
};

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
