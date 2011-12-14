#include <boost/asio/io_service.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <riak/client.hxx>
#include <riak/transports/single-serial-socket.hxx>
#include <test/tools/use-case-control.hxx>

using namespace boost;
using namespace riak::test;

void run(boost::asio::io_service& ios)
{
    ios.run();
}

int main (int argc, const char* argv[])
{
    boost::asio::io_service ios;
    std::unique_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(ios));
    boost::thread worker(std::bind(&run, std::ref(ios)));
    
    announce_with_pause("Ready to connect!");
    riak::single_serial_socket connection("localhost", 8082, ios);
    auto my_store = riak::make_client(connection, ios);
    
    announce_with_pause("Ready to store \"foodiddy howdoo!\" to item test/doc");
    RpbContent c;
    c.set_value("foodiddy howdoo!");
    auto result = my_store->bucket("test")["doc"]->put(c);
    
    announce("Waiting for operation to respond...");
    try {
        result.wait();
        if (result.has_value()) {
            announce("Store appears successful.");
        } else {
            assert(result.has_exception());
            try {
                result.get();
            } catch (const std::exception& e) {
                announce(str(format("Store failed with exception %1%: %2%.") % typeid(e).name() % e.what()));
            } catch (...) {
                announce("Store failed with a nonstandard exception.");
            }
        }
    } catch (const std::exception& e) {
        announce_with_pause(str(format("%1% reported while waiting. %2%.") % typeid(e).name() % e.what()));
    }
    
    announce_with_pause("Scenario completed.");

    work.reset();
    worker.join();
    return 0;
}
