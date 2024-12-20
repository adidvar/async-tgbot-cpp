#include <boost/test/unit_test.hpp>

#include <atgbot/awaitables/makeasync.hpp>
#include <atgbot/tools/session.hpp>
#include <functional>
#include <string>

BOOST_AUTO_TEST_SUITE(MakeAsyncAwaitableTests)

using namespace ATgBot::Awaitables;
using namespace ATgBot::Tools;

ATgBot::Coroutine CoroResult(bool& test, int& result) {

  result = co_await makeAsync(
      [&test](int arg1) {
        test = true;
        return arg1;
      }, 5);

  co_return;
}

BOOST_AUTO_TEST_CASE(CoroResultTest) {

  bool test = false;
  int result = 0;

  auto coro = CoroResult(test, result);
  bool callback = false;

  auto s = Session::create(
      std::move(coro), [&callback](auto r1) { callback = true; },
      [](auto arg) {});

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kReady);

  while (s->getStatus() != Coroutine::state_type::kDone) {
    s->tryResume();
  }

  BOOST_CHECK(callback == true);
  BOOST_CHECK(test == true);
  BOOST_CHECK(result == 5);
}

ATgBot::Coroutine CoroNoResult(bool& test) {

  co_await makeAsync(
      [&test](auto arg1) {
        test = true;
      },
      5);

  co_return;
}

BOOST_AUTO_TEST_CASE(CoroNoResultTest) {

  bool test = false;

  auto coro = CoroNoResult(test);
  bool callback = false;

  auto s = Session::create(
      std::move(coro), [&callback](auto r1) { callback = true; },
      [](auto arg) {});

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kReady);

  while (s->getStatus() != Coroutine::state_type::kDone) {
    s->tryResume();
  }

  BOOST_CHECK(callback == true);
  BOOST_CHECK(test == true);
}

BOOST_AUTO_TEST_SUITE_END();