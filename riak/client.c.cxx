/*!
 * \file
 * Implementation and C/C++ interfacing code for Riak-Cpp-C
 */
#include <boost/asio/io_service.hpp>
#include <riak/client.hxx>
#include <riak/transports/single_serial_socket.hxx>
#include <riak/client.h>

/*!
 * Hides both the C++ riak client and the Boost IO Service from the C runtime.
 */
struct riak_client_impl {
	std::shared_ptr<riak::client> client;  /*!< The primary object of interest for C API functions.

	/*!
	 * Initializes a connection to address:port, using the default Riak-Cpp transport.
	 * \param sr will be wrapped to behave exactly as the underlying C++ implomentation.
	 */
	riak_client_impl (const sibling_resolver* const sr, const char* address, const unsigned short port);

  private:
	boost::asio::io_service ios_;
};

typedef riak_client_impl client_wrapper;

//=============================================================================
extern "C" {
//=============================================================================

void riak_client_initialize (
		struct riak_client* const client,
		const sibling_resolver* sr,
		const char* address,
		const unsigned short port)
{
	assert(client != NULL);
	client->impl_ = new client_wrapper(sr, address, port);
}

//=============================================================================
}   // extern "C"

namespace {
//=============================================================================

/*!
 * Serves to facilitate the C-level sibling_resolver while matching the type of the C++-level
 * riak::sibling_resolution function.
 */
class converted_sibling_resolution
{
  public:
	converted_sibling_resolution (const sibling_resolver* const);
	std::shared_ptr<riak::object> operator () (const riak::siblings&);

  private:
  	const sibling_resolver* const c_resolver_;
};

//=============================================================================
}   // namespace (anonymous)
//=============================================================================

riak_client_impl::riak_client_impl (const sibling_resolver* const sr, const char* address, const unsigned short port)
{
	auto connection = riak::make_single_socket_transport(address, port, this->ios_);
    this->client = riak::make_client(connection, converted_sibling_resolution(sr), ios_);
}

//=============================================================================
namespace {
//=============================================================================

converted_sibling_resolution::converted_sibling_resolution (const sibling_resolver* const resolver)
  : c_resolver_(resolver)
{   }


std::shared_ptr<riak::object> converted_sibling_resolution::operator () (const riak::siblings&)
{
	// TODO: Convert C++ Protobuf objects into C protobuf objects, run resolution, and convert back.
	return std::make_shared<riak::object>();
}

//=============================================================================
}   // namespace (anonymous)
//=============================================================================
