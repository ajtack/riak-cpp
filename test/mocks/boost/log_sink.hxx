#pragma once

// Note: no such mock required unless logging is enabled.
//
#include <riak/config.hxx>
#if RIAK_CPP_LOGGING_ENABLED

#include <boost/log/attributes/attribute_value_set.hpp>
#include <boost/log/core/record_view.hpp>
#include <boost/log/sinks/sink.hpp>
#include <gmock/gmock.h>

//=============================================================================
namespace riak {
	namespace mock {
		namespace boost {
//=============================================================================

class log_sink
      : public ::boost::log::sinks::sink
{
  public:
	log_sink ();
	virtual ~log_sink ();

	MOCK_METHOD1(will_consume, bool(const ::boost::log::attribute_value_set&));
	MOCK_METHOD1(consume, void(const ::boost::log::record_view&));
	MOCK_METHOD1(try_consume, bool(const ::boost::log::record_view&));
	MOCK_METHOD0(flush, void());
};

//=============================================================================
		}   // namespac boost
	}   // namespace mock
}   // namespace riak
//=============================================================================

#endif
