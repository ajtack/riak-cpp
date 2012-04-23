/*!
 * \file
 * Defines helpful utilities which contribute to compiler compatibility.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#pragma once

//
// Check for support of the C++11 noexcept keyword. The macro RIAK_CPP_NOEXCEPT will safely expand
// to this keyword if it is available, and otherwise yield a blank.
//
#ifdef __GNUC__
#   include <features.h>
#   if __GNUC_PREREQ(4,6)
#       define RIAK_CPP_NOEXCEPT noexcept
#   else
#       define RIAK_CPP_NOEXCEPT
#  endif
#else
    // TODO: Check for clang >= 3.0 and verify.
#   define RIAK_CPP_NOEXCEPT
#endif
