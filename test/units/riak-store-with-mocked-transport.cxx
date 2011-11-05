#include <test/units/riak-store-with-mocked-transport.hxx>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

riak_store_with_mocked_transport::riak_store_with_mocked_transport ()
  : store(transport, ios)
  , closure_signal(new mock::transport::option_to_terminate_request)
{
    EXPECT_CALL(transport, deliver(_, _))
            .WillOnce(DoAll(SaveArg<1>(&request_handler), Return(closure_signal)));
}


// Defining this explicitly speeds up compilation time.
riak_store_with_mocked_transport::~riak_store_with_mocked_transport ()
{   }

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
