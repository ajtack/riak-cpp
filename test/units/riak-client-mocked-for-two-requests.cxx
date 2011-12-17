#include <test/units/riak-client-mocked-for-two-requests.hxx>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;

riak_client_mocked_for_two_requests::riak_client_mocked_for_two_requests ()
  : client(::riak::make_client(std::bind(&mock::transport::deliver, &transport, _1, _2), ios))
{
	InSequence s;
	typedef mock::transport::option_to_terminate_request mock_close_option;
	EXPECT_CALL(transport, deliver(_, _))
    	    .WillOnce(DoAll(SaveArg<1>(&request_handler_1), Return(std::bind(&mock_close_option::exercise, &close_request_1))));
	EXPECT_CALL(transport, deliver(_, _))
 	       .WillOnce(DoAll(SaveArg<1>(&request_handler_2), Return(std::bind(&mock_close_option::exercise, &close_request_2))));
}


// Defining this explicitly speeds up compilation time.
riak_client_mocked_for_two_requests::~riak_client_mocked_for_two_requests ()
{   }

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
