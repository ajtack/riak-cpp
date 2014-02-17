#pragma once
#include <gmock/gmock.h>
#include <riak/core_types.hxx>
#include <riak/error.hxx>

//=============================================================================
namespace riak {
    namespace mock {
//=============================================================================

class get_request
{
  public:
    class response_handler
    {
      public:
        MOCK_METHOD3(execute, void(
        		const std::error_code&,
        		std::shared_ptr< ::riak::object>&,
                value_updater));
    };
};

//=============================================================================
    }   // namespace mock
}   // namespace riak
//=============================================================================
