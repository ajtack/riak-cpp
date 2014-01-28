#include "captures_log_output.hxx"
#include <boost/log/core/core.hpp>
#include <test/fixtures/log/test_channel.hxx>
#include <test/matchers/has_attribute.hxx>

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

captures_log_output::captures_log_output ()
  :	log_sinks_(new std::remove_reference<decltype(log_sinks)>::type)
  ,	log_sinks(*log_sinks_)
{
	boost::log::core::get()->add_sink(log_sinks_);

	// For the sake of this sink, capture only riak-cpp client logs. All other output is ignored.
	// (order here is important: most specific matcher must go last.)
	//
	using namespace ::testing;
	ON_CALL(log_sinks, will_consume(_)).WillByDefault(Return(true));
	ON_CALL(log_sinks, will_consume(HasAttribute<riak::test::log::channel>("Channel", _)))
			.WillByDefault(Return(false));
	EXPECT_CALL(log_sinks, consume(_)).Times(AnyNumber());
}


captures_log_output::~captures_log_output ()
{
	boost::log::core::get()->remove_sink(log_sinks_);
}

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
