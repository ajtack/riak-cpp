#include <test/units/riak-store-with-mocked-transport.hxx>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

riak_store_with_mocked_transport::riak_store_with_mocked_transport ()
  : store(transport, ios)
  , closure_signal(new mock::transport::option_to_terminate_request)
{   }

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
