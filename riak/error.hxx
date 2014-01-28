#pragma once
#include <riak/compat.hxx>
#include <system_error>

//=============================================================================
namespace riak {
//=============================================================================

enum class communication_failure {
	none = 0,
	unparseable_response,
	inappropriate_response_content,

	/*!
	 * A well-behaving riak server will never yield this condition; it should generally be treated
	 * as an error. Nevertheless, this condition may actually include a useful value, so a
	 * forgiving application may choose to handle this condition differently from other errors.
	 */
	missing_vector_clock,

	response_timeout,
};

const std::error_category& communication_failure_category ();


inline std::error_code make_error_code (riak::communication_failure failure = riak::communication_failure::none) RIAK_CPP_NOEXCEPT {
	return std::error_code(static_cast<int>(failure), riak::communication_failure_category());
}

//=============================================================================
}   // namespace riak

namespace std {
//=============================================================================

template <>
struct is_error_code_enum<::riak::communication_failure>
	   : public true_type
{	};

//=============================================================================
}   // namespace std
//=============================================================================
