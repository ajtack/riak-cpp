#pragma once
#include <functional>
#include <system_error>

//=============================================================================
namespace riak {
    namespace transport {
//=============================================================================

/*!
 * Indicates to the connection pool that the associated request has ended, whether successfully
 * or otherwise. A request does not end until it is canceled using this option, and may hold any
 * number of network resources until then. Following the exercise of a cancel option, the
 * connection pool guarantees it will make no further callbacks related to the associated request,
 * and no callback will be made as part of the exercise of this option.
 *
 * The given boolean parameter is a "dirty" bit. It will be set to "true" if the request
 * is being cancelled before complete and orderly termination. In all other cases, the
 * connection used to make the request may be reused.
 *
 * This signal must be idempotent.
 */
typedef std::function<void(bool)> option_to_terminate_request;

/*!
 * A callback used to deliver a response with an error code. The error code must evaluate
 * to false unless the transport encountered an error during receive.
 */
typedef std::function<void(std::error_code, std::size_t, const std::string&)> response_handler;

/*!
 * Dispatches the given request at the next available opportunity, holding the connection as long
 * as the returned termination option is not executed and does not fall out of scope. This function
 * should return immediately, constituting asynchronous behavior.
 *
 * \param request_data is an opaque binary blob to be transmitted to the server.
 * \param on_result must always be called to indicate either failure or success, including upon
 *     destruction of the connection pool prior to resolution of a request. Multiple calls
 *     are permissible, and calls with empty payloads will affect timeouts. A conforming
 *     implementation will deliver std::network_reset in case the transport is destroyed
 *     before the request can be satisfied.
 */
typedef std::function<option_to_terminate_request(
		const std::string& request_data,
		response_handler on_result)> delivery_provider;

//=============================================================================
    }   // namespace transport
}   // namespace riak
//=============================================================================
