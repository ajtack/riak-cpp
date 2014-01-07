#pragma once
#include <boost/log/sources/channel_logger.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <test/fixtures/log/test_channel.hxx>

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

class logs_test_name
      : public ::testing::Test
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

	boost::log::sources::channel_logger<::riak::test::log::channel> logger_;
};

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
