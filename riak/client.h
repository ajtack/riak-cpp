/*!
 * \file
 * The primary entrance to riak-cpp-c; presents an interface to the Riak database.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#pragma once
#include <stdlib.h>

//=============================================================================
#ifdef __cplusplus
extern "C" {
#endif
//=============================================================================

struct riak_client_impl;

/*! The application-level handle to a Riak cluster. */
struct riak_client
{
	// The body is implemented in C++; hide this from the C runtime.
	struct riak_client_impl* impl_;
};

/*! A sibling in this wrapper is a character pointer to its data? Bullshit, let's use the C Protobuf wrapper. */
typedef char* sibling;
typedef sibling* (*sibling_resolver)(const sibling*[]);

/*!
 * Performs the initial connection to the Riak database and prepares the client for its first
 * get or put. The persnickety developer be aware: this will allocate more memory in addition
 * to the riak_client structure.
 *
 * \param client must be an allocated block the size of struct riak_client. This structure
 *     will be initialized as a clean riak object. Must not be NULL.
 */
void riak_client_initialize (
		struct riak_client* const client,
		const sibling_resolver*,
		const char* address,
		const unsigned short port);

/*!
 * Cancels all pending operations and destroys the given client handle.
 *
 * \param client must not be NULL.
 */
void riak_client_destroy (struct riak_client* const client);

//=============================================================================
#ifdef __cplusplus
}   // extern "C"
#endif
//=============================================================================
