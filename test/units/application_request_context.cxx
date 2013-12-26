#include <boost/log/sources/logger.hpp>
#include <test/actions/save_log_attribute.hxx>
#include <test/matchers/log_record_contains_attribute.hxx>
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

		EXPECT_CALL(log_sinks, consume(LogRecordContainsAttribute("Riak/ClientRequestId", A<log::request_id_type>())))
			.WillOnce(SaveLogAttribute("Riak/ClientRequestId", &id_1));
		EXPECT_CALL(log_sinks, consume(LogRecordContainsAttribute("Riak/ClientRequestId", A<log::request_id_type>())))
			.WillOnce(SaveLogAttribute("Riak/ClientRequestId", &id_2));
	}

	{	auto context_1 = new_context();  context_1.log(anywhere) << "nothing important.";
		auto context_2 = new_context();  context_2.log(anywhere) << "nothing important.";
	}

	ASSERT_NE(id_1, id_2);
}

//=============================================================================
	}   // namespace test
}   // namespace riak
//=============================================================================
