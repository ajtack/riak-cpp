#include <gtest/gtest.h>
#include <test/fixtures/riak-client-mocked-for-two-requests.hxx>
#include <test/mocks/utility/timer_factory.hxx>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
        namespace fixture {
            namespace {
//=============================================================================

std::shared_ptr<object> no_sibling_resolution (const ::riak::siblings&)
{
	ADD_FAILURE() << "Sibling resolution was triggered, when it should not have been!";
	return std::make_shared<object>();
}

//=============================================================================
            }   // namespace (anonymous)
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;

riak_client_mocked_for_two_requests::riak_client_mocked_for_two_requests ()
  :	timer_factory_mock(new NiceMock<mock::utility::timer_factory>)
  ,	client(std::bind(&mock::transport::device::deliver, &transport, _1, _2), &no_sibling_resolution, timer_factory_mock)
{
	InSequence s;
	typedef mock::transport::device::option_to_terminate_request mock_close_option;
	EXPECT_CALL(transport, deliver(_, _))
    	    .WillOnce(DoAll(
    	    		SaveArg<0>(&received_request_1),
    	    		SaveArg<1>(&request_handler_1),
    	    		Return(std::bind(&mock_close_option::exercise, &close_request_1))));
	EXPECT_CALL(transport, deliver(_, _))
 	       .WillOnce(DoAll(
 	       			SaveArg<0>(&received_request_2),
 	       			SaveArg<1>(&request_handler_2),
 	       			Return(std::bind(&mock_close_option::exercise, &close_request_2))));
}


// Defining this explicitly speeds up compilation time.
riak_client_mocked_for_two_requests::~riak_client_mocked_for_two_requests ()
{   }

//=============================================================================
        }   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
