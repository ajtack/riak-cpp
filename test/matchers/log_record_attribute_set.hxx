#pragma once
#include <gmock/gmock-matchers.h>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

/*!
 * Accepts a \c boost::log::record_view and matches on the attribute set therein contained.
 */
template <typename AttributeSetMatcher>
class LogRecordAttributeSetMatcher
      : public ::testing::MatcherInterface<const ::boost::log::record_view&>
{
  public:
	LogRecordAttributeSetMatcher (AttributeSetMatcher test = ::testing::_)
	  :	attribute_matcher_(test)
	{	}

	bool MatchAndExplain (const ::boost::log::record_view& record, ::testing::MatchResultListener* listener) const
	{
		return attribute_matcher_.MatchAndExplain(record.attribute_values(), listener);
	}

	virtual void DescribeTo (::std::ostream* os) const {
		*os << "attribute set ";
		attribute_matcher_.DescribeTo(os);
	}

	virtual void DescribeNegationTo (::std::ostream* os) const {
		*os << "attribute set ";
		attribute_matcher_.DescribeNegationTo(os);
	}

  private:
	const AttributeSetMatcher attribute_matcher_;
};


/*!
 * Accepts a \c boost::log::record_view and matches on a particular attribute type in the set of
 * attributes therein contained.
 * 
 * \code
 *	ON_CALL(log_sinks, will_consume(_)).WillByDefault(Return(true));
 *	EXPECT_CALL(log_sink, consume(LogRecordAttributeSet(Not(HasAttribute("Severity", Eq(boost::log::trivial::info))))));
 *      .Times(0);
 * \endcode
 */
template <typename AttributeSetMatcher>
::testing::Matcher<const ::boost::log::record_view&> LogRecordAttributeSet (AttributeSetMatcher test)
{
	return ::testing::MakeMatcher(new LogRecordAttributeSetMatcher<AttributeSetMatcher>(test));
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
