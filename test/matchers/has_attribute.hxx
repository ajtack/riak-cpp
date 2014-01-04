#pragma once
#include <boost/log/attributes/value_extraction.hpp>
#include <gmock/gmock-matchers.h>

//=============================================================================
namespace riak {
    namespace test {
//=============================================================================

/*!
 * Accepts a \c boost::log::attributes::attribute_value_set and detects the presence (or
 * absence) of a specific attribute of the given name and type (\c AttributeContents). The
 * matcher only fires if the attribute is both present and of the correct type.
 */
template <typename AttributeContents>
class HasAttributeMatcher
      : public ::testing::MatcherInterface<const ::boost::log::attribute_value_set&>
{
  public:
	HasAttributeMatcher (
			const std::string& target_attribute_name,
			::testing::Matcher<AttributeContents> test )
	  :	target_attribute_name_(target_attribute_name)
	  ,	test_(test)
	{	}

	bool MatchAndExplain (const ::boost::log::attribute_value_set& values, ::testing::MatchResultListener* listener) const
	{
		auto attribute = values[target_attribute_name_];
		if (!! attribute) {
			*listener << "has a logging attribute '" << target_attribute_name_ << "' and it ";
			if (auto extracted_value = boost::log::extract<AttributeContents>(attribute)) {
				return test_.MatchAndExplain(*extracted_value, listener);
			} else {
				*listener << "is not typed as " << typeid(AttributeContents).name();
				return false;
			}
		} else {
			if (values.size() == 0) {
				*listener << "has no log attributes at all.";
			} else {
				*listener << "has no logging attribute keyed by '" << target_attribute_name_ << "'. "
						<< "Present attributes include [";
				auto last_attribute = --values.end();
				for (auto attr = values.begin(); attr != last_attribute; ++attr)
					*listener << attr->first << ", ";

				*listener << last_attribute->first << "]";
			}

			return false;
		}
	}

	virtual void DescribeTo (::std::ostream* os) const {
		*os << "has an attribute '" << target_attribute_name_
		    << "' of type " << typeid(AttributeContents).name() << " that ";
		test_.DescribeTo(os);
	}

	virtual void DescribeNegationTo (::std::ostream* os) const {
		*os << "does not have an attribute '" << target_attribute_name_ << "' "
		    << "or its value is not of type " << typeid(AttributeContents).name();
	}

  private:
	const std::string target_attribute_name_;
	const ::testing::Matcher<AttributeContents> test_;
};


/*!
 * Accepts a \c boost::log::attributes::attribute_value_set and matches where it finds
 * attribute keyed by \ref target_attribute_name, the value of which matches \ref test.
 * For example, if your tests have a wired \ref boost::log::sinks::sink instance \c log_sink,
 * you can use this helper to validate which records are logged.
 * \code
 *	ON_CALL(log_sinks, will_consume(_)).WillByDefault(Return(true));
 *	EXPECT_CALL(log_sink, will_consume(HasAttribute("Severity", Eq(boost::log::trivial::error))))
 *      .Times(0);
 * \endcode
 */
template <typename AttributeContents>
::testing::Matcher<const ::boost::log::attribute_value_set&> HasAttribute (
		const std::string& target_attribute_name,
		::testing::Matcher<AttributeContents> test )
{
	return ::testing::MakeMatcher(new HasAttributeMatcher<AttributeContents>(target_attribute_name, test));
}

//=============================================================================
    }   // namespace test
}   // namespace riak
//=============================================================================
