Riak-Cpp v0.2
=============

> Unreleased.

Riak-Cpp now has Logging (#16)
------------------------
The previous release required deep understanding of the library in order to address any problems. A good server-side tool has good logging – now Riak-Cpp has it as well. Logging statements were added throughout the code using `boost::log`, verified by hand to provide a useful understanding of request progress.

The `boost::log` library is a little bit new to the scene, so it's probably even unlikely that you're using it. Fortunately, by [implementing your own sink](http://boost-log.sourceforge.net/libs/log/doc/html/log/extension.html) (admittedly a bit of work) you can pipe the output of Riak-Cpp's logging wherever you need it. Furthermore, every log line produced is tagged with the request to which it applies, the timestamp, and a severity. The output of the unit tests (see `test/units/main.cxx`) is a good example of what kind of formatting is possible.

    (test) [get_with_siblings]: Beginning test multiple_sibling_resolutions_are_correctly_handled ...
	(test) [get_with_siblings]: ==========================================================
	---- (riak/core) 2014-06-23 12:37:17.067280     INFO [r:19558c67-be15-4efd-be7f-9cd815c140fd]: GET 'a' / 'document'
	---- (riak/core) 2014-06-23 12:37:17.067473    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Parsing server response ...
	---- (riak/core) 2014-06-23 12:37:17.067536    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Found 2 siblings; attempting resolution.
	---- (riak/core) 2014-06-23 12:37:17.067598    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Resolved value has vector clock 'blah blah blah'. Transmitting ...
	---- (riak/core) 2014-06-23 12:37:17.067817    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Processing result of sibling resolution ...
	---- (riak/core) 2014-06-23 12:37:17.067874    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Value collided again upon resolution. Fetching new siblings ...
	---- (riak/core) 2014-06-23 12:37:17.068032    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Parsing server response ...
	---- (riak/core) 2014-06-23 12:37:17.068094    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Found 2 siblings; attempting resolution.
	---- (riak/core) 2014-06-23 12:37:17.068156    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Resolved value has vector clock 'blah blah blah'. Transmitting ...
	---- (riak/core) 2014-06-23 12:37:17.068400    TRACE [r:19558c67-be15-4efd-be7f-9cd815c140fd]: Processing result of sibling resolution ...
	---- (riak/core) 2014-06-23 12:37:17.068459     INFO [r:19558c67-be15-4efd-be7f-9cd815c140fd]: GET successful after applying sibling resolution.
	(test) [get_with_siblings]: ----------------------------------------------------------
	(test) [get_with_siblings]: Test Complete!

More Minor Changes
------------------
These things were fixed along the way by the author and by the community.

 1. **Less Paranoid Networking** The _Single Serial Socket_ that Riak-Cpp provides by default is still not a tremendously effective socket pool, but it will now nevertheless function faster. Unless a request terminates in exception, the connection will be re-used instead of recycled.
 2. **Fixes for Deadlocks and Crashes** thanks to the contributions of @deanoc and @mcobzarenco.

Riak-Cpp v0.1
=============

First versioned release of Riak-Cpp. The goal was correct key-value functionality with perfectly asynchronous requests.

Supported compilers:

 * VS2010
 * gcc-4.4
 * gcc-4.5

Supported libraries:

 * Boost v1.46+
 * GMock v1.5.0
 * Google Test v1.5.0
 * Protocol Buffers v2.x

Operations Supported
 
 * Get a value
 * Delete a key
 * Store a value
 
In addition, the following are supported:

 * Roll-Your-Own connection pooling (a default is provided)
 * Timeouts for store accesses of any kind
 * Storage access paremeters (R, W, etc.) for all implemented operations.
 * Asynchronous behavior, allowing performant code
 * Automatic sibling resolution
