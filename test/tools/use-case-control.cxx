#include <iostream>
#include <tools/use-case-control.hxx>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

void announce (const std::string& message)
{
    std::cerr << message << std::endl;
}

/*! Prints the given message to stderr and prompts the tester to hit the space bar. */
void announce_with_pause (const std::string& message)
{
    std::cerr << message << std::endl;
    char c;
    std::cerr << " -> (Press enter to continue…)";
    std::getchar();
    std::cerr << std::endl;
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================