/*!
 * \file
 * Implements the generic "store" interface with a simple in-memory hash table.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <boost/asio/io_service.hpp>
#include <boost/asio/connect.hpp>
#include <store.hxx>

//=============================================================================
namespace riak {
//=============================================================================

const object_access_parameters store::access_defaults = object_access_parameters()
    .with_r(2)
    .with_pr(2)
    .with_w(2)
    .with_pw(2)
    .with_dw(0)
    .with_rw(2)
    .with_basic_quorum()
    .with_notfound_ok();


store::store (const std::string& node_address, boost::asio::io_service& ios, const object_access_parameters& d)
  : access_defaults_(d),
    node_address_(node_address),
    socket_(ios)
{
    using boost::asio::ip::tcp;
    tcp::resolver resolver(ios);
    tcp::resolver::query query(node_address, "node");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::connect(socket_, endpoint_iterator);
}


store::~store ()
{   }


bucket store::bucket (const key& k)
{
    assert(this);
    return bucket::bucket(*this, k, access_defaults_);
}

//=============================================================================
}   // namespace riak
//=============================================================================
