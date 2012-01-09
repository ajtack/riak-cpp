/*!
 * \file
 * Defines core types used throughout the Riak interface.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <string>
#include <riak/riakclient.pb.h>

//=============================================================================
namespace riak {
//=============================================================================

/*! Every bucket and object is indexed by a string of this type. */
typedef std::string key;

/*! Objects retrieved from the server are delivered in this type. */
typedef RpbContent object;

/*! A vector clock is a hash, represented as a variable-length string of bytes. */
typedef std::string vector_clock;

//=============================================================================
}   // namespace riak
//=============================================================================
