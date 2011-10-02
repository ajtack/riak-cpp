/*!
 * \file
 * Defines the configuration required for correct execution of system tests.
 *
 * \author Andres Jaan Tack <ajtack@gmail.com>
 */
#pragma once
#include <boost/program_options.hpp>

namespace configuration
{
    extern boost::program_options::options_description options;
    
    /*!
     * Loads the configuration at the given path. If the file at that path doesn't exist or fails
     * to parse, throws an argument_error exception.
     */
    void process (const std::string& p);
    
    extern size_t request_timeout;  /*!< The number of milliseconds permitted before a request is considered "failed." */
};
