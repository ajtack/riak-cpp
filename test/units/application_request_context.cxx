#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <test/actions/save_log_attribute.hxx>
#include <test/matchers/has_attribute.hxx>
#include <test/matchers/log_record_attribute_set.hxx>
#include <test/fixtures/log/application_request_context.hxx>
#include <gtest/gtest.h>

using namespace ::testing;

//=============================================================================
namespace riak {
	namespace test {
//=============================================================================

using fixture::application_request_context;


TEST_F(application_request_context, yields_unique_request_ids_as_log_line_attributes)
{
	boost::log::sources::logger anywhere;
	riak::log::request_id_type id_1, id_2;

	{	InSequence s;
		namespace log = riak::log;

		EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute("Riak/ClientRequestId", A<log::request_id_type>()))))
			.WillOnce(SaveLogAttribute("Riak/ClientRequestId", &id_1));
		EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute("Riak/ClientRequestId", A<log::request_id_type>()))))
			.WillOnce(SaveLogAttribute("Riak/ClientRequestId", &id_2));
	}

	{	auto context_1 = new_context();  context_1.log(anywhere) << "nothing important.";
		auto context_2 = new_context();  context_2.log(anywhere) << "nothing important.";
	}

	ASSERT_NE(id_1, id_2);
}


TEST_F(application_request_context, applies_indicated_severity_to_log_lines)
{
	using riak::log::severity;
	boost::log::sources::severity_logger<severity> anywhere;

	{	InSequence s;
		EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute<severity>("Severity", Eq(severity::info)))));
		EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute<severity>("Severity", Eq(severity::trace)))));
	}

	auto context_1 = new_context();  context_1.log(anywhere, severity::info) << "nothing important.";
	auto context_2 = new_context();  context_2.log(anywhere, severity::trace) << "nothing important.";
}


TEST_F(application_request_context, applies_info_level_severity_to_log_lines_by_default)
{
	using riak::log::severity;
	boost::log::sources::severity_logger<severity> anywhere;

	{	InSequence s;
		EXPECT_CALL(log_sinks, consume(LogRecordAttributeSet(HasAttribute<severity>("Severity", Eq(severity::info)))));
	}

	auto context_1 = new_context();  context_1.log(anywhere) << "nothing important.";
}

//=============================================================================
	}   // namespace test
}   // namespace riak
//=============================================================================
