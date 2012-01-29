Riak-Cpp (better name pending™) is a _work-in-progress_ asychronous [Riak](http://basho.com/products/riak-overview/) client library for advanced C++ compilers.

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

To fetch or store an object using Riak-Cpp, you need to have four things set up:

 1. **A sibling-resolution strategy.** Sibling-resolution is a natural property of an eventually-consistent store like Riak. Riak-Cpp forces you to think about this problem up front. If you do not expect siblings, be sure to log errors when sibling resolution is encountered on your application.

   The shape of a sibling-resolution handler is as the below. You will of course want this to depend on your data type.

        std::shared_ptr<riak::object> random_sibling_resolution (const ::riak::siblings&)
        {
            std::cout << "Siblings being resolved!" << std::endl;
            auto new_content = std::make_shared<riak::object>();
            new_content->set_value("<result of sibling resolution>");
	        return new_content;
	    }

    You will apply sibling-resolution immediately upon construction of the Riak client.

 2. **A result callback for the action you perform.** Riak-Cpp is asynchronous in all cases. As C++11 evolves, we will be able to use lambdas to shorten some cases, but for now we suggest defining functions like the below to handle your get and put responses.

	    void print_object_value (const std::error_code& error, std::shared_ptr<riak::object> object, riak::value_updater&)
	    {
	        if (not error) {
	            if (!! object)
	                std::cout << "Fetch succeeded! Value is: " << object->value() << std::endl;
	            else
	                std::cout << "Fetch succeeded! No value found." << std::endl;
	        } else {
	            std::cout << "Could not receive the object from Riak due to a network or server error." << std::endl;
	        }
	    }

	The third parameter to this handler can be used to store values back to a key that you have fetched. Because we need to trigger sibling resolution, it is forced by Riak-Cpp that every Put occur only following a Get.

 3. **A connection pool.** For experimentation, we currently provide a low-performance single-socket connection "pool" at your disposal. You can use it by giving the host and port as below.

	    boost::io_service ios;
	    auto connection = riak::make_single_socket_transport("localhost", 8082, ios);

	For any high-performance application, you will need your own connection pool. See `transport.hxx` for details on what interfaces you need to implement.

 4. **A boost::io_service to run request timeouts.** This may eventually be replaced, but for the time being you will need to run (and thus watch) a `boost::io_service` instance that Riak-Cpp will use to run `deadline_timer`s and ensure that your requests eventually time out.

With all of the above in place, we can write a simple program that fetches a value from the server.

	int main (int argc, const char* argv[])
	{
	    boost::asio::io_service ios;
	    auto connection = riak::make_single_socket_transport("localhost", 8082, ios);
	    auto my_store = riak::make_client(connection, &random_sibling_resolution, ios);

	    my_store->get_object("test", "doc", std::bind(&print_object_value, _1, _2, _3));
	    ios.run();
	    return 0;
	}

Implementation Status
=====================

We reiterate: This library is a work in progress. As of this moment, the following operations are supported:
 
 * Get a value
 * Delete a key
 * Store a value
 
In addition, the following are supported:

 * Roll-Your-Own connection pooling (a default is provided)
 * Timeouts for store accesses of any kind
 * Storage access paremeters (R, W, etc.) for all implemented operations.
 * Asynchronous behavior, allowing performant code
 * Automatic sibling resolution

Be sure to check out the Github [Issues](http://github.com/ajtack/riak-cpp/issues) to see what's planned next for development.

Contributing
============

Your help is welcome! Pull requests will be the format of contribution to this project, along with _some_ supporting test cases (which needn't be automatic, just reproducible: see the readme).
