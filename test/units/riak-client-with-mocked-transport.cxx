#include <test/units/riak-client-with-mocked-transport.hxx>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;

riak_client_with_mocked_transport::riak_client_with_mocked_transport ()
  : client(::riak::make_client(std::bind(&mock::transport::deliver, &transport, _1, _2), ios))
{
	typedef mock::transport::option_to_terminate_request mock_close_option;
    EXPECT_CALL(transport, deliver(_, _))
            .WillOnce(DoAll(SaveArg<1>(&request_handler), Return(std::bind(&mock_close_option::exercise, &closure_signal))));
}


// Defining this explicitly speeds up compilation time.
riak_client_with_mocked_transport::~riak_client_with_mocked_transport ()
{   }

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
