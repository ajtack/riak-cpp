#include <test/fixtures/deleting_client.hxx>

//=============================================================================
namespace riak {
    namespace test {
    	namespace fixture {
//=============================================================================

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

deleting_client::deleting_client ()
  : response_handler(std::bind(&mock_response_handler::execute, &response_handler_mock, _1))
{   }


// Defining this explicitly speeds up compilation time.
deleting_client::~deleting_client ()
{   }

//=============================================================================
		}   // namespace fixture
    }   // namespace test
}   // namespace riak
//=============================================================================
