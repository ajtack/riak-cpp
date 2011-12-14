#include <test/units/riak-client-with-mocked-transport.hxx>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

riak_client_with_mocked_transport::riak_client_with_mocked_transport ()
  : client(::riak::make_client(transport, ios))
  , closure_signal(new mock::transport::option_to_terminate_request)
{
    EXPECT_CALL(transport, deliver(_, _))
            .WillOnce(DoAll(SaveArg<1>(&request_handler), Return(closure_signal)));
}


// Defining this explicitly speeds up compilation time.
riak_client_with_mocked_transport::~riak_client_with_mocked_transport ()
{   }

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
