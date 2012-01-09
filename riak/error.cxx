#include <cassert>
#include <riak/error.hxx>

//=============================================================================
namespace riak {
	namespace {
//=============================================================================

class server_error_category
      : public std::error_category
{
  public:
    virtual const char* name () const {
    	return "riak::server_error";
    }

    virtual std::string message (int ev) const {
    	auto ec = static_cast<errc>(ev);
    	assert(ec == errc::response_was_nonsense);
    	return "A target Riak server responded to a request nonsensically.";
    }
} the_server_error_category;

//=============================================================================
	}   // namespace (anonymous)
//=============================================================================

const errc errc::no_error(0);
const errc errc::response_was_nonsense(1);

std::error_code make_server_error(const errc code)
{
	return std::error_code(static_cast<int>(code), server_error());
}

const std::error_category& server_error ()
{
	return the_server_error_category;
}

//=============================================================================
}   // namespace riak
//=============================================================================
