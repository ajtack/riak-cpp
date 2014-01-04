#pragma once
#include <boost/log/sources/channel_logger.hpp>
#include <gtest/gtest.h>
#include <memory>

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

	enum class channel
	{	
		test_output,
		test_log_decoration,
		blank_line,
	};

  private:
	class scope;
	std::unique_ptr<scope> scope_;
	boost::log::sources::channel_logger<channel> logger_;
};

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
