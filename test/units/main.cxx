#if RIAK_CPP_LOGGING_ENABLED
#   include <boost/log/expressions.hpp>
#   include <boost/log/expressions/formatters/date_time.hpp>
#   include <boost/log/expressions/formatters/if.hpp>
#   include <boost/log/expressions/keyword.hpp>
#   include <boost/log/support/date_time.hpp>
#   include <boost/log/utility/formatting_ostream.hpp>
#   include <boost/log/utility/setup/common_attributes.hpp>
#   include <boost/log/utility/setup/file.hpp>
#   include <boost/uuid/uuid_io.hpp>
#endif

#include <gtest/gtest.h>
#include <riak/log.hxx>
#include <test/fixtures/log/logs_test_name.hxx>

// Where logging is enabled, shape test output beautifully.
//
#if RIAK_CPP_LOGGING_ENABLED

//=============================================================================
namespace riak {
    namespace log {
//=============================================================================

using boost::log::basic_formatting_ostream;

template <typename ch, typename traits>
basic_formatting_ostream<ch, traits>& operator<< (basic_formatting_ostream<ch, traits>& output, const severity& level)
{
    output << std::setw(8);

    switch (level) {
        case severity::error:    output << "ERROR";    break;
        case severity::warning:  output << "WARNING";  break;
        case severity::info:     output << "INFO";     break;
        case severity::trace:    output << "TRACE";    break;
        default:                 output << "???";      break;
    }

    return output;
}


template <typename ch, typename traits>
basic_formatting_ostream<ch, traits>& operator<< (basic_formatting_ostream<ch, traits>& output, const channel& chan)
{
    switch (chan) {
        case channel::core:     output << "riak/core";  break;
        case channel::network:  output << "riak/net";   break;
        default:                output << "riak/??";    break;
    }

    return output;
}

//=============================================================================
    }   // namespace log
}   // namespace riak

namespace {
//=============================================================================

namespace log = boost::log;
namespace expr = boost::log::expressions;

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", riak::log::severity);
BOOST_LOG_ATTRIBUTE_KEYWORD(client_log_channel, "Channel", riak::log::channel);
BOOST_LOG_ATTRIBUTE_KEYWORD(test_log_channel, "Channel", riak::test::log::channel);
BOOST_LOG_ATTRIBUTE_KEYWORD(request_id, "Riak/ClientRequestId", riak::log::request_id_type);


void divert_all_logs_to_file (const std::string& filename)
{
    log::add_common_attributes();
    log::register_simple_formatter_factory<riak::log::request_id_type, char>("Riak/ClientRequestId");

    auto backend = boost::make_shared<log::sinks::text_file_backend>(log::keywords::file_name = filename);
    auto format_as_text = boost::make_shared<log::sinks::synchronous_sink<log::sinks::text_file_backend>>(backend);

    namespace test = riak::test;
    format_as_text->set_formatter( expr::stream
         << expr::if_ (test_log_channel.or_default(test::log::channel::test_output) != test::log::channel::blank_line) [
                expr::stream
                     << expr::if_ (not client_log_channel) [
                            expr::stream << "(test) [" << expr::attr<std::string>("TestCase") << "]"
                        ] .else_ [
                            expr::stream
                                << "---- "
                                << '(' << client_log_channel << ") "
                                << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f ")
                                << severity << ' '
                                << "[r:" << request_id << ']'
                        ]
                     << ": " << expr::message
            ] .else_ [
                expr::stream   // Blank line
            ]
        );

    log::core::get()->add_sink(format_as_text);
}

//=============================================================================
}   // namespace (anonymous)
//=============================================================================

#endif

int main (int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

#   if RIAK_CPP_LOGGING_ENABLED
        divert_all_logs_to_file("units.log");
#   endif

    return RUN_ALL_TESTS();
}
