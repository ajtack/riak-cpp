#pragma once
#include <gmock/gmock.h>
#include <riak/core_types.hxx>
#include <riak/error.hxx>

//=============================================================================
namespace riak {
    namespace mock {
//=============================================================================

class delete_request
{
  public:
    class result_handler
    {
      public:
        MOCK_METHOD3(execute, void(const std::error_code&, const ::riak::key&, const ::riak::key&));
    };
};

//=============================================================================
    }   // namespace mock
}   // namespace riak
//=============================================================================
