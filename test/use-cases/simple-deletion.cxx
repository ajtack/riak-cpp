#include <boost/asio/io_service.hpp>
#include <boost/format.hpp>
#include <functional>
#include <riak/client.hxx>
#include <riak/transports/single_serial_socket.hxx>
#include <test/tools/use-case-control.hxx>

using namespace boost;
using namespace riak::test;
using namespace std::placeholders;

std::shared_ptr<riak::object> no_sibling_resolution (const ::riak::siblings&);

void handle_deletion_result (const std::error_code& error)
{
    if (not error)
        announce("Deletion successful!");
    else
        announce("Deletion failed!");
}


int main (int argc, const char* argv[])
{
    boost::asio::io_service ios;
    
    announce_with_pause("Connecting!");
    auto connection = riak::make_single_socket_transport("localhost", 8082, ios);
    auto my_store = riak::make_client(connection, &no_sibling_resolution, ios);
    
    announce_with_pause("Ready to delete item test/doc");
    my_store->delete_object("test", "doc", std::bind(&handle_deletion_result, _1));
    ios.run();
    return 0;
}


std::shared_ptr<riak::object> no_sibling_resolution (const ::riak::siblings&)
{
    // Deletion never triggers sibling resolution in Riak-Cpp.
    assert(false);
}
