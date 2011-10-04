/*!
 * \file
 * Implementation as per header. Currently excludes real functionality (play with hash table).
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#include <bucket.hxx>

//=============================================================================
namespace riak {
//=============================================================================

object bucket::operator[] (const ::riak::key& k)
{
    return object(store_, *this, k);
}

//=============================================================================
}   // namespace riak
//=============================================================================
