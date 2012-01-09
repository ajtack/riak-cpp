#pragma once
#include <functional>

//=============================================================================
namespace riak {
    namespace transport {
//=============================================================================

/*!
 * Indicates to the connection pool that the associated request has ended, whether successfully
 * or otherwise. The connection pool may cancel any ongoing requests by the mechanism it
 * wishes. Following the exercise of a cancel option, the connection pool guarantees it
 * will make no further callbacks related to the associated request, and no callback will
 * be made as part of the exercise.
 *
 * This signal must be idempotent.
 */
typedef std::function<void()> option_to_terminate_request;

/*!
 * A callback used to deliver a response with an error code. The error code must evaluate
 * to false unless the transport encountered an error during receive.
 */
typedef std::function<void(std::error_code, std::size_t, const std::string&)> response_handler;

/*!
 * Dispatches the given request at the next available opportunity. This function
 * may optionally return immediately, constituting asynchronous behavior.
 *
 * \param r is an opaque binary blob to be transmitted to the server.
 * \param h must always be called to indicate either failure or success, including upon
 *     destruction of the connection pool prior to resolution of a request. Multiple calls
 *     are permissible, and calls with empty payloads will affect timeouts. A conforming
 *     implementation will deliver std::network_reset in case the transport is destroyed
 *     before the request can be satisfied.
 */
typedef std::function<option_to_terminate_request(const std::string&, response_handler)> delivery_provider;

//=============================================================================
    }   // namespace transport
}   // namespace riak
//=============================================================================
