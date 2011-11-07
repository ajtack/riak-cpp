Testing the Riak C++ Client
===========================

A database client library is a crucial piece of infrastructure, and thus must be tested. The author
is also a huge fan of automated tests. Riak C++ library tests, however, are not at this stage automatic,
because the we believe the overhead of reliably (let alone cross-platform) supporting such automated
tests to be at a cost greater than any fathomable benefit. The likelihood is that these tests will deal
very deeply with client library behavior.

We Do Not Automate Potential Failure Cases
------------------------------------------

bq. "But what could be so hard that we can't automate these tests?!"

First off, stop. Nobody is saying it is _impossible_ to automate these tests. We are saying only that it
is not _cost-effective_ to do so. Intrepid test-minded people are welcome, of course, to prove us wrong.

Second off, notice that we're not punting on test automation entirely. We are, in fact, only punting on the problem of expressing each and every possible _failure_ automatically. The scenarios themselves are a different story; keep reading.

What Makes Test Automation Difficult
------------------------------------

The things that make automated testing of this library difficult include primarily:

 1. **Using "real" Riak nodes.** It is meaningless to test this library against a mock implementation. If the library does not work against a real Riak store, there is no amount of functional testing that will make that okay.
 2. **Timeout regressions.** These take forever to diagnose with reasonable values, and are difficult to diagnose reliably when timeouts are very short.
 3. **Network race conditions.** This class of failures are difficult to express in tests to begin with. In this case of network-related problems, they require platform-specific methods to delay and proceed individual requests and responses.
 4. **Asymmetric network failures.** Unexpected failures will appear in two ways: either the transmission of the request will fail (server never got it) or the response will fail (server couldn't get back to us). The best way to control these is with manual tools like `iptables`.
 5. **Failures across connections.** All the problems above are exacerbated when a library such as this is extended to pool resources across multiple sockets and TCP connections.
 
Again we point out that the tools we use to instigate these failures _can_ be automated: it is _possible_. You can totally control iptables programmatically through shell commands. No question there. We reiterate that this is a question of resource allocation (for now, perhaps).

Making Testing Easy
-------------------

We present here our approach to testing the Riak C++ client library. The core principles of this approach are this:

 1. **Plan for Tests against Adverse Network Conditions.**
 
    All of the interesting problem cases above come down to network control. Above all, we want to facilitate the testing of this library in the presence of network faults.

 2. **Build a library of test scenarios.**
    Such a library will include flows that an application will use. As examples, consider the following stories:
    
    1. "Store an object, read it back, then store again with an updated vector clock."
    2. "Read an object, store a new value, read the latest value again."
    3. "Store an object with low replication, read back with low quorum constraints."
    4. "Provide a sibling resolution function. Store two values for an object with different client IDs and NULL vector clocks. Read one value back."
    
    The point of producing these scenarios is to automate the things a tester will do _all the time_, which is repeating the scenarios that the application is actually using. If this library is complete, any application failure can be broken down to one of (or a chain of) these examples, and a tester can verify the library by manually inducing "world" conditions surrounding the failure. This leads to our next tenet.
    
 3. **Support the manual induction of adverse network conditions during a test.**
    
    A tester will need some facility to delay operations. During these delays, he will install or remove various network and system conditions (e.g. inducing a blocked TCP connection) before ultimately cueing the tests to proceed. The most basic form of such an enabling feature is the ability to prompt for user input before proceeding to the next step of a scenario.
    
 4. **Automate verification steps that are exhausting to do manually.**
 
    Those adverse conditions which take too long to "manually" apply, or are highly prone to user error. For instance, generating data for a load test and verifying the sanity of the results is a good candidate for some automated script, even if that script itself is run manually. _We never say no to things that save dramatic amounts of time._

Test Resource Structure
-----------------------

In the `test/` folder you will find three sub-directories supporting our testing story.

 1. **`failure-scenarios/`** – These text documents are essentially individual test reproduction steps. Markdown is suggested, but anything expressive and accessible is permitted. The authors are welcome to contribute by writing tests and organizing them by topic.
 2. **`use-cases/`** – The contents of this folder support our desire to express application scenarios. From this set of use-cases, we should be able to support all of the situations in the `failure_scenarios` folder.
 3. **`tools/`** – These programmatic tools may be used either by parts of individual use-cases (to make them more easily controllable) or by individual failure scenarios (to induce particular situations programmatically). Dead tools should be removed with prejudice – they are dead because we don't need them to reproduce failures.
 4. **`units/`** — The parts of the Riak library which are exposed directly to code provided by a user are tested here against all manner of nonsense return values, but not against any thread-safety requirements.
