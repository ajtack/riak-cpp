#include "timer.hxx"

//=============================================================================
namespace riak {
	namespace mock {
		namespace utility {
//=============================================================================

timer::timer ()
{
	using ::testing::_;
	using ::testing::Return;
	using ::testing::SaveArg;

	// Timeout behavior is distracting in the common case.
	ON_CALL( *this, run_on_timeout(_, _) ).WillByDefault(Return());
	ON_CALL( *this, cancel() ).WillByDefault(Return(1));
}

// Defining this explicitly speeds up compilation time.
timer::~timer ()
{	}

//=============================================================================
		}   // namespace utility
	}   // namespace mock
}   // namespace riak
//=============================================================================
