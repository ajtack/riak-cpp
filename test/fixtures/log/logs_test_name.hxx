#pragma once

#ifdef _WIN32
    // Solves a problem in which BOOST_PP_... is missing in included files.
#   include <boost/preprocessor/repetition/enum_binary_params.hpp>
#endif

#include <riak/config.hxx>
#if RIAK_CPP_ENABLE_LOGGING
#	include <boost/log/sources/channel_logger.hpp>
#endif

#include <gtest/gtest.h>
#include <memory>
#include <test/fixtures/log/test_channel.hxx>

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

/*!
 * If logging is enabled, the log output will contain additional log lines produced by the test framework
 * itself. These lines will include the running test name in particular.
 *
 * If logging is not compiled in, this fixture has no effect.
 */
#if RIAK_CPP_ENABLE_LOGGING
	class logs_test_name
		  :	public ::testing::Test
	{
		static bool first_test_has_run_;
		
	  public:
		logs_test_name ();
		virtual ~logs_test_name ();
		virtual void SetUp ();
		virtual void TearDown ();

	  private:
		class scope;
		std::unique_ptr<scope> scope_;

		boost::log::sources::channel_logger< ::riak::test::log::channel> logger_;
	};
#else
	class logs_test_name
		  :	public ::testing::Test
	{	};
#endif

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
