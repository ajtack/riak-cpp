#pragma once
#include <riak/core_types.hxx>
#include <riak/error.hxx>

//=============================================================================
namespace riak {
//=============================================================================

typedef std::function
		< void(const std::error_code&, const ::riak::key&, const ::riak::key&) >
		delete_response_handler;

typedef std::function<void(const std::error_code&)> put_response_handler;

/*!
 * Such a function promises to deliver the given value using correct update semantics. In terms of
 * a Riak store, this means remembering the bucket, key, and vector clock (most importantly) of a
 * fetched object.
 */
typedef std::function<void(const std::shared_ptr<object>&, put_response_handler&)> value_updater;
typedef std::function<void(const std::error_code&, std::shared_ptr<object>&, value_updater&)> get_response_handler;

//=============================================================================
}   // namespace riak
//=============================================================================
