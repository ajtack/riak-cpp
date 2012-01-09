#pragma once
#include <riak/riakclient.pb.h>

//=============================================================================
namespace riak {
//=============================================================================

typedef ::RpbContent sibling;
typedef ::google::protobuf::RepeatedPtrField<sibling> siblings;

/*!
 * Such a function will receive a list of siblings (concurrently-occurring values) and respond with
 * a value that can be considered a descendent of all of them. Such a value will most likely become
 * the only value in the system, overwriting all other siblings available.
 *
 * A sibling resolution may not raise any exception. It is the system programmer's responsibility
 * to ensure that data stored in the system is monotonic and thus resolveable.
 *
 * \param sibs is guaranteed to be of size > 1.
 * \return The index of the content among the given siblings that is the "resolved" content. This
 *    must be a valid index within siblings, else the behavior is undefined.
 */
typedef std::function<size_t(const siblings& sibs)> sibling_resolution;

//=============================================================================
}   // namespace riak
//=============================================================================
