#pragma once
#include <riak/config.hxx>
#include <boost/shared_ptr.hpp>
#include <test/mocks/boost/log_sink.hxx>

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

/*!
 * Fixtures of this type will direct any log output NOT from the test framework itself into the
 * provided mock \c log_sinks. This is not availed where logging is compiled-out.
 */
#if RIAK_CPP_LOGGING_ENABLED
	class captures_log_output
	{
	  public:
		captures_log_output ();
		~captures_log_output ();

	  private:
		boost::shared_ptr< ::testing::NiceMock<mock::boost::log_sink>> log_sinks_;

	  public:
		/*!
		 * A representative of all log sinks attached to the boost logging core, with no filtering
		 * in place. Useful for capturing logging output from a request context instance.
		 */
		::testing::NiceMock<mock::boost::log_sink>& log_sinks;
	};
#else
	class captures_log_output
	{	};
#endif

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
