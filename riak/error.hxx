#pragma once
#include <system_error>

//=============================================================================
namespace riak {
//=============================================================================

enum class errc {
	no_error = 0,
	response_was_nonsense = 1,
};

const std::error_category& server_error ();

std::error_code make_server_error(const errc code = errc::no_error);

//=============================================================================
}   // namespace riak
//=============================================================================
