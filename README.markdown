Riak-Cpp (better name pending™) is a _work-in-progress_ Asychronous [Riak](http://basho.com/products/riak-overview/) client library for advanced C++ compilers.

Requirements
============

For the purpose of more rapid development, as well as hopefully a positive API experience, Riak-Cpp's library and compiler requirements are a little bit high. Please take this into consideration, if you consider Riak for your own use.

 * **A C++11 compiler, at the level of GCC 4.4 or higher**
 
    Riak uses both C++11 language features (e.g. type inference) and headers (e.g. `<chrono>`, and `std::bind` from `<functional>`). This decision was made to reduce the stringency of dependencies on Boost.
 
 * **Boost v1.46 (ish)**
 
    Notable libraries used include Asio, Optional and Thread. During development, Boost v1.46 is used for testing. We may reduce the requirements to earlier versions at some later point, but as of today that has not been tested.
 
 * **SCons (for building)**
 
    SCons made the buildscript writing process faster, for now. We are open to other systems. In the meantime, compiling shouldn't be hard if you need to write your own build scripts.

 * **Google Test and Google Mock (for building)**
 
    We have found here a use for unit testing, which is greatly facilitated by these two libraries.

Quick Start Guide
=================

> Riak is like sex. Let's stop talking about it and do it. —Anonymous

The following program demonstrates how to attach to a Riak store and fetch a key. It also hints at how you might start doing this asynchronously.

    #include <boost/asio/io_service.hpp>
    #include <boost/format.hpp>
    #include <boost/thread.hpp>
    #include <functional>
    #include <source/store.hxx>
    
    using namespace boost;
    
    void run(boost::asio::io_service& ios)
    {
        ios.run();
    }
    
    int main (int argc, const char* argv[])
    {
        // Start a thread that handles socket activity.
        boost::asio::io_service ios;
        std::unique_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(ios));
        boost::thread worker(std::bind(&run, std::ref(ios)));
    
        // Connect to a Riak Store. Note that Riak-Cpp uses the Protocol Buffers API to access Riak.
        riak::store my_store("localhost", 8082, ios);
        
        // Fetch a key synchronously using Futures. In HTTP, this would be the object at test/doc.
        auto result = my_store["test"]["doc"]->fetch();
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
            }
        }
        
        return 0;
    }

Implementation Status
=====================

We reiterate: This library is a work in progress. As of this moment, the following operations are supported:
 
 * Get a value
 * Delete a key
 * Store a value (NB! only after an explicit Fetch on the same object reference)
 
In addition, the following are supported:

 * Timeouts for store accesses of any kind
 * Storage access paremeters (R, W, etc.) for all implemented operations.
 * Asynchronous behavior

The following might constitute a roughly-ordered backlog of features yet to be implemented:

 1. Implementation of `Put` operations (with cached vector clocks)
 2. Callback-based asynchronous behavior (as opposed to boost:futures -- should remove the dependency on `boost::thread`)
 3. Connection pooling support + Thread safety
 4. Callback-based sibling resolution hooks
 5. Implementation of `MapReduce` operations
 6. …

Contributing
============

Your help is welcome! Pull requests will be the format of contribution to this project, along with _some_ supporting test cases (which needn't be automatic, just reproducible: see the readme).
