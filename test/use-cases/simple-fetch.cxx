#include <boost/asio/io_service.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <riak/store.hxx>
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
    
    announce_with_pause("Ready to fetch item test/doc");
    auto result = my_store->bucket("test")["doc"]->fetch();
    
    announce("Waiting for operation to respond...");
    try {
        result.wait();
        if (result.has_value() and not result.get()) {
            announce("Fetch appears successful. Value was not found.");
        } else if (result.has_value()) {
            auto val = result.get();
            announce(str(format("Fetch appears successful. Value is: %1%") % val->Get(0).value()));
        } else {
            assert(result.has_exception());
            try {
                result.get();
            } catch (const std::exception& e) {
                announce(str(format("Fetch reported exception %1%: %2%.") % typeid(e).name() % e.what()));
            } catch (...) {
                announce("Fetch produced a nonstandard exception.");
            }
        }
    } catch (const std::exception& e) {
        announce_with_pause(str(format("Fetch reported exception %1%: %2%.") % typeid(e).name() % e.what()));
    }
    
    announce_with_pause("Scenario completed.");

    work.reset();
    worker.join();
    return 0;
}
