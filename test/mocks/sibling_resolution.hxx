#pragma once
#include <gmock/gmock.h>
#include <riak/sibling_resolution.hxx>

//=============================================================================
namespace riak {
    namespace mock {
//=============================================================================

class sibling_resolution
{
  public:
  	MOCK_METHOD1(evaluate, std::shared_ptr<object>(const ::riak::siblings&));
};

//=============================================================================
	}   // namespace mock
}   // namespace riak
//=============================================================================
