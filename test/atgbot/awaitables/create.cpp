#include <boost/test/unit_test.hpp>

#include <atgbot/awaitables/create.hpp>
#include <atgbot/tools/session.hpp>
#include <functional>
#include <string>

BOOST_AUTO_TEST_SUITE(CreateAwaitableTests)

using namespace ATgBot::Awaitables;
using namespace ATgBot::Tools;

template <typename T>
ATgBot::Coroutine Coro(T t) {

  co_await t;

  co_return;
}

ATgBot::Coroutine TestCoro() {

  co_return;
}

BOOST_AUTO_TEST_CASE(CallbackWorkTest) {

  auto coro = Coro(createCoro(TestCoro()));
  bool works = false;

  auto s = Session::create(
      std::move(coro), [](auto r1) {}, [&works](auto r1) { works = true; });


  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kReady);

  s->tryResume();

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kReady);
  BOOST_CHECK(works == true);
}

BOOST_AUTO_TEST_SUITE_END();
