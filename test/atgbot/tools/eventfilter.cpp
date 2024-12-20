#include <boost/test/unit_test.hpp>

#include <functional>
#include <string>
#include <atgbot/tools/eventfilter.hpp>

BOOST_AUTO_TEST_SUITE(EventFilterTests)

using namespace ATgBot::Tools;

BOOST_AUTO_TEST_CASE(DefaultState)
{
    EventFilter<int> filter;
    BOOST_CHECK(!filter.check(42));
}

BOOST_AUTO_TEST_CASE(EnabledState)
{
    EventFilter<int> filter;
    filter.setEnabled(true);
    BOOST_CHECK(filter.check(42));
}

BOOST_AUTO_TEST_CASE(DisabledState)
{
    EventFilter<int> filter;
    filter.setEnabled(false);
    BOOST_CHECK(!filter.check(42));
}

BOOST_AUTO_TEST_CASE(AdditionalFilter)
{
    EventFilter<int> filter;
    filter.setEnabled(true);
    filter.setAdditionalFilter([](int value) { return value % 2 == 0; });
    BOOST_CHECK(filter.check(4));
    BOOST_CHECK(!filter.check(5));
}

BOOST_AUTO_TEST_CASE(AdditionalFilterWithDisabled)
{
    EventFilter<int> filter;
    filter.setEnabled(false);
    filter.setAdditionalFilter([](int value) { return value % 2 == 0; });
    BOOST_CHECK(!filter.check(4));
    BOOST_CHECK(!filter.check(5));
}

BOOST_AUTO_TEST_CASE(ChangingFilters)
{
    EventFilter<int> filter;
    filter.setEnabled(true);
    filter.setAdditionalFilter([](int value) { return value > 0; });
    BOOST_CHECK(filter.check(10));
    BOOST_CHECK(!filter.check(-5));
    filter.setAdditionalFilter([](int value) { return value > 100; });
    BOOST_CHECK(filter.check(101));
    BOOST_CHECK(!filter.check(50));
}

BOOST_AUTO_TEST_CASE(ComplexPredicate)
{
    EventFilter<std::string> filter;
    filter.setEnabled(true);
    filter.setAdditionalFilter([](const std::string& value) {
        return value.find("test") != std::string::npos;
    });
    BOOST_CHECK(filter.check("unittest"));
    BOOST_CHECK(!filter.check("example"));
}

BOOST_AUTO_TEST_SUITE_END()
