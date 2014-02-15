#include "application_request_context.hxx"

//=============================================================================
namespace riak {
	namespace test {
		namespace fixture {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;

application_request_context::application_request_context ()
{	}


application_request_context::~application_request_context ()
{	}


riak::application_request_context application_request_context::new_context ()
{
	return riak::application_request_context(oap_, rfp_);
}

//=============================================================================
		}   // namespace fixture
	}   // namespace test
}   // namespace riak
//=============================================================================
