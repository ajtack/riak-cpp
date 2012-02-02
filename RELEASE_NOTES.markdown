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
