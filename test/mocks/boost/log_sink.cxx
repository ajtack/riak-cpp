// Note: no implementation required unless logging is enabled.
//
#include <riak/config.hxx>
#if RIAK_CPP_LOGGING_ENABLED

#include "log_sink.hxx"

//=============================================================================
namespace riak {
	namespace mock {
		namespace boost {
//=============================================================================

// Speed up compilation.
log_sink::log_sink ()
  :	sink(false)
{	}


// Speed up compilation.
log_sink::~log_sink ()
{	}

//=============================================================================
		}   // namespace boost
	}   // namespace mock
}   // namespace riak
//=============================================================================

#endif
