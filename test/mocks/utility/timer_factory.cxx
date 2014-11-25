#include "timer_factory.hxx"
#include "timer.hxx"

//=============================================================================
namespace riak {
	namespace mock {
		namespace utility {
			namespace {
//=============================================================================

// Timeout behavior is distracting in the common case.
mock::utility::timer* new_mock_timer () {
	using namespace ::testing;
	return new NiceMock<mock::utility::timer>;
}

//=============================================================================
			}   // namespace (anonymous)
//=============================================================================

timer_factory::timer_factory ()
{
	using namespace ::testing;
	ON_CALL( *this, __create() ).WillByDefault(InvokeWithoutArgs(&new_mock_timer));
}

// Defining this explicitly speeds up compilation time.
timer_factory::~timer_factory ()
{	}

//=============================================================================
		}   // namespace utility
	}   // namespace mock
}   // namespace riak
//=============================================================================
