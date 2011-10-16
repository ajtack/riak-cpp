/*!
 * \file
 * Defines a set of helper tools for tests, which are designed to facilitate manual testing of time-sensitive
 * interleavings.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <string>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

void announce (const std::string& message);

/*! Prints the given message to stderr and prompts the tester to hit the space bar. */
void announce_with_pause (const std::string& message);

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================