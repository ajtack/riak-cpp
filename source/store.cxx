/*!
 * \file
 * Implements the generic "store" interface with a simple in-memory hash table.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <boost/asio/connect.hpp>
#include <boost/lexical_cast.hpp>
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


const request_failure_parameters store::failure_defaults = request_failure_parameters()
    .with_response_timeout(3000)
    .with_retries_permitted(1);


store::store (
        const std::string& node_address,
        uint16_t port,
        boost::asio::io_service& ios,
        const request_failure_parameters& fp,
        const object_access_parameters& d)
  : access_defaults_(d),
    request_failure_defaults_(fp),
    node_address_(node_address),
    socket_(ios),
    ios_(ios)
{
    using boost::asio::ip::tcp;
    tcp::resolver resolver(ios);
    tcp::resolver::query query(node_address, boost::lexical_cast<std::string>(port));
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


void store::transmit_request(const std::string& body, response_handler& h, size_t timeout)
{
    std::shared_ptr<request_with_timeout> request(new request_with_timeout(body, timeout, socket_, h, ios_));
    request->dispatch();
}

//=============================================================================
}   // namespace riak
//=============================================================================
