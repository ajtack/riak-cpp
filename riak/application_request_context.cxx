#include <riak/application_request_context.hxx>

//=============================================================================
namespace riak {
//=============================================================================

boost::uuids::random_generator application_request_context::new_uuid;


application_request_context::application_request_context (
		const object_access_parameters& oap,
		const request_failure_parameters& rfp)
  :	access_overrides(oap)
  ,	request_failure_defaults(rfp)
  ,	request_id(new_uuid())
{	}


application_request_context::~application_request_context ()
{	}

//=============================================================================
}   // namespace riak
//=============================================================================
