#pragma once

//=============================================================================
namespace riak {
//=============================================================================

typedef ::RpbContent sibling;
typedef ::google::protobuf::RepeatedPtrField<sibling> siblings;

/*!
 * Such a function will receive a list of siblings (concurrently-occurring values) and respond with
 * a value that can be considered a descendent of all of them. Such a value will most likely become
 * the only value in the system, overwriting all other siblings available.
 */
typedef std::function<RpbContent(const siblings&)> sibling_resolution;

//=============================================================================
}   // namespace riak
//=============================================================================
