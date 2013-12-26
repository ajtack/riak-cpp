#pragma once
#include <boost/log/attributes/value_extraction.hpp>
#include <gmock/gmock-generated-actions.h>

//=============================================================================
namespace riak {
	namespace test {
//=============================================================================

/*!
 * Acts as \c ::testing::SaveArg<int>(), but with the matched \c boost::log::record_view
 * (singular -- use \c ::testing::WithArg<0>) retrieves the log attribute keyed at \ref name and
 * extracts a value of decltype(*target_pointer). If either attribute keying or type matching
 * fails, will cause a test failure.
 */
ACTION_P2(SaveLogAttribute, name, target_pointer)
{
	typedef typename std::remove_pointer<target_pointer_type>::type target_pointee_type;
	auto value_extracted = boost::log::extract<target_pointee_type>(arg0.attribute_values()[name]);
	ASSERT_TRUE(!! value_extracted);
	*target_pointer = value_extracted.get();
}

//=============================================================================
	}   // namespace test
}   // namespace riak
//=============================================================================
