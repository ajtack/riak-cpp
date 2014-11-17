#include "timer_factory.hxx"
#include "timer.hxx"

//=============================================================================
namespace riak {
	namespace mock {
		namespace utility {
//=============================================================================

timer_factory::timer_factory ()
{
	using namespace ::testing;

	// Timeout behavior is distracting in the common case.
	auto new_mock_timer = []() { return new NiceMock<mock::utility::timer>; };
	ON_CALL( *this, __create() ).WillByDefault(InvokeWithoutArgs(new_mock_timer));
}

// Defining this explicitly speeds up compilation time.
timer_factory::~timer_factory ()
{	}

//=============================================================================
		}   // namespace utility
	}   // namespace mock
}   // namespace riak
//=============================================================================
