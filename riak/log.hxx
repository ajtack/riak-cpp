#pragma once

namespace boost {
	namespace uuids { struct uuid; }
}

//=============================================================================
namespace riak {
	namespace log {
//=============================================================================

typedef boost::uuids::uuid request_id_type;

enum class severity
{
	trace,
	info,
	warning,
	error,
};

enum class channel
{
	core,
	network,
};

//=============================================================================
	}   // namespace log
}   // namespace riak
//=============================================================================
