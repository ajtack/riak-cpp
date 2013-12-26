#include "logs_test_name.hxx"
#include <boost/log/trivial.hpp>
#include <boost/log/attributes/function.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

// Note: Google Test runs in a single thread. No concurrency protection needed.
bool logs_test_name::first_test_has_run_ = false;

//=============================================================================
			namespace {
//=============================================================================

typedef decltype(boost::log::add_scoped_thread_attribute("name", boost::log::attribute())) scoped_attribute;
namespace log = boost::log;
namespace attrs = boost::log::attributes;

scoped_attribute test_case_attribute_from (const ::testing::TestInfo* current_test)
{
	std::function<std::string()> test_case_name = std::bind(&::testing::TestInfo::test_case_name, current_test);
	return log::add_scoped_thread_attribute("TestCase", attrs::make_function(test_case_name));
}

scoped_attribute test_name_attribute_from (const ::testing::TestInfo* current_test)
{
	std::function<std::string()> test_name = std::bind(&::testing::TestInfo::name, current_test);
	return log::add_scoped_thread_attribute("TestName", attrs::make_function(test_name));
}

//=============================================================================				
			}   // namespace (anonymous)
//=============================================================================

class logs_test_name::scope
{
  public:
	scope (const ::testing::TestInfo* test_info)
	  :	case_scope_(test_case_attribute_from(test_info))
	  ,	test_scope_(test_name_attribute_from(test_info))
	{	}

  private:
	scoped_attribute case_scope_;
	scoped_attribute test_scope_;
};


logs_test_name::logs_test_name ()
{	}


logs_test_name::~logs_test_name ()
{	}


void logs_test_name::SetUp ()
{
	auto current_test = ::testing::UnitTest::GetInstance()->current_test_info();
	scope_.reset(new scope(current_test));
	if (first_test_has_run_)
		BOOST_LOG_TRIVIAL(trace);

	BOOST_LOG_TRIVIAL(info) << "Beginning test " << current_test->name() << " ...";
	BOOST_LOG_TRIVIAL(trace) << "==========================================================";
}


void logs_test_name::TearDown ()
{
	BOOST_LOG_TRIVIAL(trace) << "----------------------------------------------------------";
	BOOST_LOG_TRIVIAL(info) << "Test Complete!";
	scope_.reset();
	first_test_has_run_ = true;
}

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
