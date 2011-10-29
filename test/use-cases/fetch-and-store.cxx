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
    riak::store my_store("localhost", 8082, ios);
    
    announce_with_pause("Ready to fetch item test/doc");
    auto cached_object = my_store["test"]["doc"];
    auto result = cached_object->fetch();
    
    announce("Waiting for operation to respond...");
    result.wait();
    if (result.has_value()) {
        announce("Fetch appears successful.");
        RpbContent c;
        c.set_value("oogaboogah");
        
        announce_with_pause("Ready to store 'oogaboogah' to item test/doc");
        auto result = cached_object->put(c);
        
        announce("Waiting for operation to respond...");
        result.wait();
        if (result.has_value()) {
            announce("Store appears successful.");
        } else {
            assert(result.has_exception());
            try {
                result.get();
            } catch (const std::exception& e) {
                announce(str(format("Store reported exception %1%: %2%.") % typeid(e).name() % e.what()));
            } catch (...) {
                announce("Fetch produced a nonstandard exception.");
            }
        }
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
    
    announce_with_pause("Scenario completed.");

    work.reset();
    worker.join();
    return 0;
}
