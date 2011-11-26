#include <test/units/riak-store-mocked-for-two-requests.hxx>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

riak_store_mocked_for_two_requests::riak_store_mocked_for_two_requests ()
  : store(transport, ios)
  , close_request_1(new mock::transport::option_to_terminate_request)
  , close_request_2(new mock::transport::option_to_terminate_request)
{
	InSequence s;
	EXPECT_CALL(transport, deliver(_, _))
    	    .WillOnce(DoAll(SaveArg<1>(&request_handler_1), Return(close_request_1)));
	EXPECT_CALL(transport, deliver(_, _))
 	       .WillOnce(DoAll(SaveArg<1>(&request_handler_2), Return(close_request_2)));
}


// Defining this explicitly speeds up compilation time.
riak_store_mocked_for_two_requests::~riak_store_mocked_for_two_requests ()
{   }

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
