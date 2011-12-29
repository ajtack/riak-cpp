#pragma once
#include <riak/core_types.hxx>
#include <riak/error.hxx>

//=============================================================================
namespace riak {
//=============================================================================

typedef std::function<void(const std::error_code&, const ::riak::key&, const ::riak::key&)> delete_response_handler;

//=============================================================================
}   // namespace riak
//=============================================================================
