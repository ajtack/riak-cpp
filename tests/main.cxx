#include <cassert>
#include <configuration.hxx>
#include <UnitTest++/UnitTest++.h>

int main (int argc, const char* argv[])
{
    if (argc == 2) {
        configuration::process(argv[1]);
        return UnitTest::RunAllTests();
    } else {
        std::cerr << "Usage: " << argv[0] << " <config filename>" << std::endl;
        return -1;
    }
}
