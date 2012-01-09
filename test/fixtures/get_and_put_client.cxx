#include <test/fixtures/get_and_put_client.hxx>

//=============================================================================
namespace riak {
    namespace test {
    	namespace fixture {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

get_and_put_client::get_and_put_client ()
  : get_response_handler(std::bind(&mock::get_request::response_handler::execute, &get_response_handler_mock, _1, _2, _3))
  , put_response_handler(std::bind(&mock::put_request::response_handler::execute, &put_response_handler_mock, _1))
{   }


// Defining this explicitly speeds up compilation time.
get_and_put_client::~get_and_put_client ()
{   }

//=============================================================================
		}   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
