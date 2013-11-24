#pragma once
#include <boost/asio/io_service.hpp>
#include <string>
#include <riak/transport.hxx>

//=============================================================================
namespace riak {
	namespace transport {
//=============================================================================

/*!
 * Produces a transport providing serial delivery of requests along one socket at a time. All of
 * these requests will act under the given client_id.
 */
transport::delivery_provider make_single_socket_transport (
        const std::string& address,
        uint16_t port,
        boost::asio::io_service& ios);

//=============================================================================
	}   // namespace transport
}   // namespace riak
//=============================================================================
