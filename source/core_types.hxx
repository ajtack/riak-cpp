/*!
 * \file
 * Defines core types used throughout the Riak interface.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <string>

//=============================================================================
namespace riak {
//=============================================================================

/*! Every kind of object is indexed by a string, in an object store. */
typedef std::string key;

//=============================================================================
}   // namespace riak
//=============================================================================
