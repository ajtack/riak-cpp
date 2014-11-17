#pragma once
#include <gmock/gmock.h>
#include <riak/utility/timer.hxx>
#include <riak/utility/timer_factory.hxx>

//=============================================================================
namespace riak {
	namespace mock {
		namespace utility {
//=============================================================================

class timer_factory
      : public ::riak::utility::timer_factory
{
public:
	timer_factory ();
	~timer_factory ();

	virtual std::unique_ptr<::riak::utility::timer> create () {
		return std::unique_ptr<::riak::utility::timer>(__create());
	}

	// Google decided they don't like unique_ptr, so the rest of us are stuck doing this.
	MOCK_METHOD0( __create, ::riak::utility::timer* () );
};

//=============================================================================
		}   // namespace utility
	}   // namespace mock
}   // namespace riak
//=============================================================================
