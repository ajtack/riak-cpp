#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <list>
#include <memory>
#include <riak/transport.hxx>

//=============================================================================
namespace riak {
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
}   // namespace riak
//=============================================================================
