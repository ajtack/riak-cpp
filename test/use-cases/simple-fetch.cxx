#include <boost/asio/io_service.hpp>
#include <boost/format.hpp>
#include <functional>
#include <riak/client.hxx>
#include <riak/transports/single-serial-socket.hxx>
#include <test/tools/use-case-control.hxx>

using namespace boost;
using namespace riak::test;
using namespace std::placeholders;

std::shared_ptr<riak::object> no_sibling_resolution (const ::riak::siblings&);

void print_object_value (const std::error_code& error, std::shared_ptr<riak::object> object, riak::value_updater&)
{
    if (not error) {
        if (!! object)
            announce(str(format("Fetch succeeded! Value is: %1%") % object->value()));
        else
            announce(str(format("Fetch succeeded! No value found.")));
    } else {
        announce("Could not receive the object from Riak due to a hard error.");
    }
}


int main (int argc, const char* argv[])
{
    boost::asio::io_service ios;
    
    announce_with_pause("Connecting!");
    auto connection = riak::make_single_socket_transport("localhost", 8082, ios);
    auto my_store = riak::make_client(connection, &no_sibling_resolution, ios);
    
    announce_with_pause("Ready to fetch item test/doc");
    my_store->get_object("test", "doc", std::bind(&print_object_value, _1, _2, _3));
    ios.run();
    return 0;
}


std::shared_ptr<riak::object> no_sibling_resolution (const ::riak::siblings&)
{
    announce("Siblings being resolved!");
    auto garbage = std::make_shared<riak::object>();
    garbage->set_value("<result of sibling resolution>");
    garbage->set_content_type("text/plain");
    return garbage;
}
