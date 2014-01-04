#include "application_request_context.hxx"
#include <boost/log/core/core.hpp>
#include <test/matchers/has_attribute.hxx>

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;

application_request_context::application_request_context ()
  :	log_sinks_(new std::remove_reference<decltype(log_sinks)>::type)
  ,	log_sinks(*log_sinks_)
{
	boost::log::core::get()->add_sink(log_sinks_);

	// For the sake of this sink, ignore test output to logs, allow everything else.
	// (order here is important: most specific matcher must go last.)
	//
	using namespace ::testing;
	typedef logs_test_name::channel channel;
	ON_CALL(log_sinks, will_consume(_)).WillByDefault(Return(true));
	ON_CALL(log_sinks, will_consume(HasAttribute<channel>("Channel", A<channel>())))
			.WillByDefault(Return(false));
}


application_request_context::~application_request_context ()
{
	boost::log::core::get()->remove_sink(log_sinks_);
}


riak::application_request_context application_request_context::new_context ()
{
	transport::delivery_provider mock_transport_device =
			std::bind(&mock::transport::device::deliver, std::ref(transport_), _1, _2);

	return riak::application_request_context(oap_, rfp_, mock_transport_device, ios_);
}

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
