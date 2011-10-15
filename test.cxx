#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>
#include <functional>
#include <source/store.hxx>

void run(boost::asio::io_service& ios)
{
    ios.run();
}

int main (int argc, const char* argv[])
{
    boost::asio::io_service ios;
    
    std::unique_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(ios));
    std::shared_ptr<riak::store> my_store(new riak::store("localhost", 8082, ios));
    boost::thread worker(std::bind(&run, std::ref(ios)));
    
    auto result = my_store->bucket("test").unmap("LX1zMpo2wIBPO4od9VLVPUwdb20");
    result.wait();
    assert(result.has_value());
    std::cerr << "Deleted successfully!" << std::endl;

    work.reset();
    worker.join();
    return 0;
}