#include <cassert>
#include <riak/compat.hxx>
#include <riak/error.hxx>
#include <string>

//=============================================================================
namespace riak {
	namespace {
//=============================================================================

class communication_failure_category
      : public std::error_category
{
  public:
	virtual const char* name () const RIAK_CPP_NOEXCEPT {
		return "communication failure with riak server";
	}

	virtual std::string message (int ev) const {
		switch (static_cast<communication_failure>(ev)) {
		  case communication_failure::none: return "No network failure observed.";
		  case communication_failure::unparseable_response: return "The Riak server responded with unparseable or corrupt data.";
		  case communication_failure::inappropriate_response_content: return "The Riak server sent a response that was parseable, but with invalid content.";
		  case communication_failure::response_timeout: return "The response took too long to arrive.";
		  default: assert(false);
		}
	}
} the_communication_failure_category;

//=============================================================================
	}   // namespace (anonymous)
//=============================================================================

const std::error_category& communication_failure_category ()
{
	return the_communication_failure_category;
}

//=============================================================================
}   // namespace riak
//=============================================================================
