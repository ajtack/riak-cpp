#pragma once
#include <boost/asio/io_service.hpp>
#include <riak/application_request_context.hxx>
#include <test/fixtures/logs_test_name.hxx>
#include <test/fixtures/log/captures_log_output.hxx>
#include <test/mocks/transport.hxx>

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

class application_request_context
	  : public logs_test_name
	  , public captures_log_output
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
};

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
