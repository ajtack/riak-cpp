#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cstdatomic>
#include <functional>
#include <riak/client.hxx>
#include <riak/transports/single_serial_socket.hxx>
#include <test/tools/use-case-control.hxx>

using namespace boost;
using namespace riak::test;
using namespace std::placeholders;

boost::mutex mutex_;
std::atomic<std::size_t> successful_reads;
std::atomic<std::size_t> successful_writes;
std::atomic<std::size_t> unsuccessful_reads;
std::atomic<std::size_t> unsuccessful_writes;

void print_stats (boost::asio::deadline_timer& timer, const boost::system::error_code& error) {
    if (error != boost::asio::error::operation_aborted) {
		timer.expires_from_now(boost::posix_time::milliseconds(100));
        timer.async_wait(std::bind(&print_stats, std::ref(timer), _1));
		std::cout << "----" << std::endl;
		std::cout << successful_writes << " " << successful_reads << std::endl;
		std::cout << unsuccessful_writes << " " << unsuccessful_reads << std::endl;
	}
}

std::shared_ptr<riak::object> no_sibling_resolution (const ::riak::siblings&);

void handle_put_result (const std::error_code& error)
{	
	boost::unique_lock<boost::mutex> protect(mutex_);
    if (not error)
        ++successful_writes;
    else
    	++unsuccessful_writes;
}


void print_object_value (
        const std::string& new_value,
        const std::error_code& error,
        std::shared_ptr<riak::object> object,
        riak::value_updater& update_value)
{
	boost::unique_lock<boost::mutex> protect(mutex_);
    if (not error) {
        ++successful_reads;
        protect.unlock();
        auto new_content = std::make_shared<riak::object>();
        new_content->set_value(new_value);
        new_content->set_content_type("text/plain");
        riak::put_response_handler put_handler = std::bind(&handle_put_result, _1);
        update_value(new_content, put_handler);
    } else {
        ++unsuccessful_reads;
    }
}


int main (int argc, const char* argv[])
{
    boost::asio::io_service ios;
    boost::asio::deadline_timer period(ios);
    period.expires_from_now(boost::posix_time::milliseconds(100));
    period.async_wait(std::bind(&print_stats, std::ref(period), _1));
    
    announce_with_pause("Connecting!");
    auto connection = riak::make_single_socket_transport("localhost", 8082, ios);
    auto my_store = riak::make_client(connection, &no_sibling_resolution, ios);

    std::size_t max_outstanding_queries;
    if (argc > 1)
	    max_outstanding_queries = lexical_cast<std::size_t>(argv[1]);
	else
		max_outstanding_queries = 2000;

    announce_with_pause("Ready to begin load test...");
    for (std::size_t query = 0; query < max_outstanding_queries; ++query) {
    	// std::cout << "sending!" << std::endl;
    	if (argc > 2)
	        my_store->get_object("test", lexical_cast<std::string>(query % 1000), std::bind(&print_object_value, argv[2], _1, _2, _3));
	    else
	        my_store->get_object("test", lexical_cast<std::string>(query % 1000), std::bind(&print_object_value, "Some sense!", _1, _2, _3));
    }
    
    ios.run();
    return 0;
}


std::shared_ptr<riak::object> no_sibling_resolution (const ::riak::siblings& sibs)
{
    std::cerr << sibs.size() << " siblings being resolved: ";
    std::cerr << sibs.Get(0).value();
    for (std::size_t i = 1; i < sibs.size(); ++i) {
    	std::cerr << ", " << sibs.Get(i).value();
    }
    std::cerr << std::endl;
    auto garbage = std::make_shared<riak::object>();
    garbage->set_value("<result of sibling resolution>");
    garbage->set_content_type("text/plain");
    return garbage;
}
