#include <test/fixtures/getting_client.hxx>

//=============================================================================
namespace riak {
    namespace test {
    	namespace fixture {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

getting_client::getting_client ()
  : response_handler(std::bind(&mock_response_handler::execute, &response_handler_mock, _1, _2, _3))
{   }


// Defining this explicitly speeds up compilation time.
getting_client::~getting_client ()
{   }

//=============================================================================
		}   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
