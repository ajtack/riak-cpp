#include <functional>
#include <test/fixtures/get_with_siblings.hxx>
#include <test/mocks/utility/timer_factory.hxx>

//=============================================================================
namespace riak {
    namespace test {
        namespace fixture {
//=============================================================================

using namespace testing;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

get_with_siblings::get_with_siblings ()
  : timer_factory_mock(new NiceMock<mock::utility::timer_factory>)
  , client(transport.as_delivery_provider(),
           std::bind(&mock::sibling_resolution::evaluate, &sibling_resolution, _1),
           timer_factory_mock)
  , response_handler(std::bind(&::riak::mock::get_request::response_handler::execute, &response_handler_mock, _1, _2, _3))
{
    typedef mock::transport::device::option_to_terminate_request mock_close_option;
    ON_CALL(transport, deliver(_, _))
            .WillByDefault(DoAll(
                    SaveArg<1>(&send_from_server), 
                    Return(std::bind(&mock_close_option::exercise, &closure_signal))));
}


// Defining this explicitly speeds up compilation time.
get_with_siblings::~get_with_siblings ()
{   }

//=============================================================================
        }   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
