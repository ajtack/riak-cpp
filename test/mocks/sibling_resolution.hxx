#pragma once
#include <gmock/gmock.h>

//=============================================================================
namespace riak {
    namespace mock {
//=============================================================================

class sibling_resolution
{
  public:
  	MOCK_METHOD1(evaluate, ::riak::sibling(const ::riak::siblings&));
};

//=============================================================================
	}   // namespace mock
}   // namespace riak
//=============================================================================
