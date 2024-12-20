#include <boost/test/unit_test.hpp>

#include <optional>
#include <string>
#include <functional>

#include <atgbot/tools/eventfilter.hpp>
#include <atgbot/tools/eventqueue.hpp>

BOOST_AUTO_TEST_SUITE(EventQueueTests)

using namespace ATgBot::Tools;

BOOST_AUTO_TEST_CASE(DefaultState) {
  ATgBot::Tools::EventQueue<int> queue;
  BOOST_CHECK(queue.empty());
  BOOST_CHECK(queue.hasChanges());
}

BOOST_AUTO_TEST_CASE(PushAndPop) {
  ATgBot::Tools::EventQueue<int> queue;
  queue.push(10);
  BOOST_CHECK(queue.empty());  // Default filter blocks all elements

  ATgBot::Tools::EventFilter<int> filter;
  filter.setEnabled(true);
  queue.setFilter(filter);
  queue.push(10);
  BOOST_CHECK(!queue.empty());
  auto elem = queue.pop();
  BOOST_REQUIRE(elem.has_value());
  BOOST_CHECK_EQUAL(elem.value(), 10);
  BOOST_CHECK(queue.empty());
}

BOOST_AUTO_TEST_CASE(AdditionalFilter) {
  ATgBot::Tools::EventQueue<int> queue;
  ATgBot::Tools::EventFilter<int> filter;
  filter.setEnabled(true);
  filter.setAdditionalFilter([](int value) { return value % 2 == 0; });
  queue.setFilter(filter);

  queue.push(4);
  queue.push(5);
  BOOST_CHECK(!queue.empty());
  auto elem = queue.pop();
  BOOST_REQUIRE(elem.has_value());
  BOOST_CHECK_EQUAL(elem.value(), 4);
  BOOST_CHECK(queue.empty());
}

BOOST_AUTO_TEST_CASE(ClearQueue) {
  ATgBot::Tools::EventQueue<int> queue;
  ATgBot::Tools::EventFilter<int> filter;
  filter.setEnabled(true);
  queue.setFilter(filter);

  queue.push(1);
  queue.push(2);
  queue.clear();
  BOOST_CHECK(queue.empty());
}

BOOST_AUTO_TEST_CASE(HasChanges) {
  ATgBot::Tools::EventQueue<int> queue;
  BOOST_CHECK(queue.hasChanges());

  ATgBot::Tools::EventFilter<int> filter;
  filter.setEnabled(true);
  queue.setFilter(filter);
  BOOST_CHECK(queue.hasChanges());

  queue.resetChanges();
  BOOST_CHECK(!queue.hasChanges());

  queue.push(10);
  BOOST_CHECK(!queue.hasChanges());
}

BOOST_AUTO_TEST_SUITE_END()