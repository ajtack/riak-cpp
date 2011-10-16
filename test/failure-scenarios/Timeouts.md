Testing Timeouts
================

The (X) primary test cases for timeouts are as follows:

 * Zero retries configured, timeout 5s => one timeout should fail the request.
 * One retry configured, timeout 5s => waiting 6s should yield a success
 * One retry configured, timeout 5s => waiting 11s should fail the request.

Timeouts are easy to reproduce with the simple-deletion scenario.