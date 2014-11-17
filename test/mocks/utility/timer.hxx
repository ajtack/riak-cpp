#pragma once
#include <gmock/gmock.h>
#include <riak/utility/timer.hxx>

//=============================================================================
namespace riak {
	namespace mock {
		namespace utility {
//=============================================================================

class timer
      : public ::riak::utility::timer
{
  public:
	timer ();
	~timer ();

	MOCK_METHOD2( run_on_timeout,
			void(const std::chrono::milliseconds&, std::function<void(const std::error_code&)>) );

	MOCK_METHOD0( cancel, std::size_t() );
};

//=============================================================================
		}   // namespace utility
	}   // namespace mock
}   // namespace riak
//=============================================================================
