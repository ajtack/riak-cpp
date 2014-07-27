/*!
 * \file
 * Defines empty shadows of all of the logging classes needed to build riak-cpp.
 * 
 * This file may not be included anywhere unless logging is disabled.
 */
#pragma once
#include <riak/log.hxx>
#include <boost/parameter/name.hpp>
#include <boost/parameter/preprocessor.hpp>
#include <string>

namespace boost {
	namespace log {
		template <typename Value>
		int add_value (const std::string&, Value) {
			return 0;
		}

		namespace keywords {
			BOOST_PARAMETER_NAME((severity, tag) severity)
		}
	}
}

//=============================================================================
namespace riak {
	namespace log {
		namespace null {
//=============================================================================

namespace tag { class logger; }

class log_record
{	};

class logger {
  public:
	BOOST_PARAMETER_MEMBER_FUNCTION(
		(log_record),
		open_record,
		::boost::log::keywords::tag,
		(required)
		(optional
			(severity, *, riak::log::severity::info)
		)
	)
	{
		return log_record();
	}
};

//=============================================================================
		}	// namespace null
	}	// namespace log
}	// namespace riak
//=============================================================================
