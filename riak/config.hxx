/*!
 * \file
 * Defines all compilation-time configuration options for riak-cpp.
 */
#pragma once

//
// Logging is enabled by default, because one really wants logging for server-side tools.
// However, it can be disabled just as easily.
//
#ifndef RIAK_CPP_LOGGING_ENABLED
#	define RIAK_CPP_LOGGING_ENABLED 1
#endif
