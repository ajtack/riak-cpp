#include <gtest/gtest.h>
#include <test/fixtures/riak-client-with-mocked-transport.hxx>

using namespace ::testing;

//=============================================================================
namespace riak {
    namespace test {
        namespace fixture {
        	namespace {
//=============================================================================

const std::shared_ptr<object> no_sibling_resolution (const ::riak::siblings&)
{
    ADD_FAILURE() << "Sibling resolution was triggered, when it should not have been!";
    return std::make_shared<RpbContent>();
}

//=============================================================================
    		}   // namespace (anonymous)
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

riak_client_with_mocked_transport::riak_client_with_mocked_transport ()
  : client(::riak::make_client(std::bind(&mock::transport::deliver, &transport, _1, _2), &no_sibling_resolution, ios))
  , response_handler(std::bind(&::riak::mock::get_request::response_handler::execute, &response_handler_mock, _1, _2, _3))
{
    typedef mock::transport::option_to_terminate_request mock_close_option;
    EXPECT_CALL(transport, deliver(_, _))
            .WillOnce(DoAll(SaveArg<1>(&send_from_server), Return(std::bind(&mock_close_option::exercise, &closure_signal))));
}


// Defining this explicitly speeds up compilation time.
riak_client_with_mocked_transport::~riak_client_with_mocked_transport ()
{   }

//=============================================================================
        }   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
