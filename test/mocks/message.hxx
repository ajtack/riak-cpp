#pragma once
#include <gmock/gmock.h>
#include <riak/message.hxx>

//=============================================================================
namespace riak {
	namespace mock {
		namespace message {
//=============================================================================

class handler
{
  public:
	handler ();
	~handler ();

	MOCK_METHOD3(handle, bool(std::error_code, std::size_t, const std::string&));

	/*!
	 * A helper for quick representation of a message handler in its native callback from.
	 */
	::riak::message::handler as_message_handler () {
		using namespace std::placeholders;
		return std::bind(&handler::handle, this, _1, _2, _3);
	}
};

//=============================================================================
		}   // namespace message
	}   // namespace mock
}   // namespace riak
//=============================================================================
