/*!
 * \file
 * Includes the definitions of various configuration options as they will be parsed.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#include <configuration.hxx>
#include <fstream>
#include <stdexcept>

//=============================================================================
namespace configuration {
//=============================================================================

static boost::program_options::options_description option_definitions ();

boost::program_options::options_description options = option_definitions();
size_t request_timeout;

void process(const std::string& p)
{
    using namespace boost::program_options;
    
    std::ifstream file(p);
    if (file) {
        request_timeout = 50;  // Boost is broken on macports. :^)
        // variables_map vm;
        //         boost::program_options::options_description options = option_definitions();
        //         store(parse_config_file(file, options), vm);
        //         notify(vm);
    } else {
        throw std::invalid_argument("The given configuration file does not exist.");
    }
}


boost::program_options::options_description option_definitions ()
{
    using namespace boost::program_options;
    
    options_description desc("Storage Test Options");
    desc.add_options()
    ("request-timeout", value<size_t>(&configuration::request_timeout)->default_value(50),
        "The number of milliseconds permitted before a request is considered \"failed.\"");
    return desc;
}

//=============================================================================
}   // namespace configuration
//=============================================================================
