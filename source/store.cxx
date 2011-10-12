/*!
 * \file
 * Implements the generic "store" interface with a simple in-memory hash table.
 *
 * \author Andres Jaan Tack <andres.jaan.tack@eesti.ee>
 */
#include <arpa/inet.h>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
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
    namespace {
//=============================================================================

typedef boost::asio::buffers_iterator<boost::asio::streambuf::const_buffers_type> iterator;

/*!
 * Intended to delay reception until the response constitutes a complete Riak command of the form
 * | Rest-of-Message Length (32 bits) | Message Code (8 bits) | Message Body |.
 */
std::pair<iterator, bool> response_complete (const iterator begin, const iterator end)
{
    if (end - begin >= sizeof(uint32_t) + sizeof(char)) {
        uint32_t encoded_length = *reinterpret_cast<const uint32_t*>(&*begin);
        uint32_t data_length = ntohl(encoded_length);
        if (end - (begin + sizeof(encoded_length)) >= data_length) {
            iterator new_beginning = begin + sizeof(encoded_length) + data_length;   // day breaks!
            return std::make_pair(new_beginning, true);
        } else {
            return std::make_pair(begin, false);
        }
    } else {
        return std::make_pair(begin, false);
    }
}

//=============================================================================
    }   // namespace anonymous
//=============================================================================

namespace system = boost::system;
namespace asio = boost::asio;
using std::placeholders::_1;
using std::placeholders::_2;


void store::transmit_request(const std::string& r, std::shared_ptr<boost::asio::streambuf> b, response_handler& h)
{
    typedef std::function<void(const system::error_code&, size_t)> write_handler;
    write_handler on_write = std::bind(&store::handle_write, shared_from_this(), b, h, _1, _2);
    asio::async_write(socket_, asio::buffer(r), on_write);
}


void store::handle_write (
        std::shared_ptr<boost::asio::streambuf> buffer,
        response_handler handler,
        const system::error_code& error,
        size_t bytes_transferred)
{
    if (not error) {
        asio::async_read_until(socket_, *buffer, &response_complete, std::bind(handler, _1, _2, buffer));
    } else {
        handler(error, 0, buffer);
    }
}

//=============================================================================
}   // namespace riak
//=============================================================================
