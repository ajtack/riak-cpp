#pragma once
#include <gmock/gmock.h>
#include <riak/core_types.hxx>
#include <riak/error.hxx>

//=============================================================================
namespace riak {
    namespace mock {
//=============================================================================

class put_request
{
  public:
    class response_handler
    {
      public:
        MOCK_METHOD1(execute, void(const std::error_code&));
    };
};

//=============================================================================
    }   // namespace mock
}   // namespace riak
//=============================================================================
