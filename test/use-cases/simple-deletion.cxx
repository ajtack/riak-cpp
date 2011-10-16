#include <boost/asio/io_service.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <source/store.hxx>
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
    std::shared_ptr<riak::store> my_store(new riak::store("localhost", 8082, ios));
    
    announce_with_pause("Ready to delete item test/LX1zMpo2wIBPO4od9VLVPUwdb20");
    auto result = my_store->bucket("test").unmap("LX1zMpo2wIBPO4od9VLVPUwdb20");
    
    announce("Waiting for deletion to respond...");
    try {
        result.wait();
        if (result.has_value())
            announce("Deletion appears successful.");
        else {
            assert(result.has_exception());
            try {
                result.get();
            } catch (const std::exception& e) {
                announce(str(format("Deletion reported exception %1%: %2%.") % typeid(e).name() % e.what()));
            } catch (...) {
                announce("Deletion produced a nonstandard exception.");
            }
        }
    } catch (const std::exception& e) {
        announce_with_pause(str(format("Deletion reported exception %1%: %2%.") % typeid(e).name() % e.what()));
    }
    
    announce_with_pause("Scenario completed.");

    work.reset();
    worker.join();
    return 0;
}