/*!
 * \file
 * Defines helpful utilities which contribute to compiler compatibility.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#pragma once

#ifndef __has_feature
#	define __has_feature(x) 0
#endif

//
// Check for support of the C++11 noexcept keyword. The macro RIAK_CPP_NOEXCEPT will safely expand
// to this keyword if it is available, and otherwise yield a blank.
//
#ifdef __GNUC__
#   if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 6) || __has_feature(cxx_noexcept)
#       define RIAK_CPP_NOEXCEPT noexcept
#   else
#       define RIAK_CPP_NOEXCEPT
#  endif
#else
#   define RIAK_CPP_NOEXCEPT
#endif
