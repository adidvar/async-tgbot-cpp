#include <boost/test/unit_test.hpp>

#include <atgbot/awaitables/callbackquery.hpp>
#include <atgbot/tools/session.hpp>
#include <functional>
#include <string>

BOOST_AUTO_TEST_SUITE(CBQueryAwaitableTests)

using namespace ATgBot::Awaitables;
using namespace ATgBot::Tools;

template <typename T>
ATgBot::Coroutine Coro(T t) {

  co_await t;

  co_return;
}

BOOST_AUTO_TEST_CASE(DefaultDehaviourWithCoro) {
  EventFilter<TgBot::CallbackQuery::Ptr> filter;
  filter.setEnabled(true);

  CBQueryAwaitable a(filter);
  auto coro = Coro(a);

  auto s = Session::create(std::move(coro), [](auto r1) {}, [](auto r1) {});

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kReady);

  s->callback_queue.resetChanges();
  s->tryResume();
  BOOST_CHECK(s->callback_queue.hasChanges());

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kWait);

  s->tryResume();

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kWait);

  s->callback_queue.push(nullptr);

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kReady);

  s->tryResume();

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kDone);
  BOOST_CHECK(s->callback_queue.empty());
}

BOOST_AUTO_TEST_CASE(DefaultDehaviourWithCoroWithNullFilter) {
  EventFilter<TgBot::CallbackQuery::Ptr> filter{};

  CBQueryAwaitable a(filter);
  auto coro = Coro(a);

  auto s = Session::create(std::move(coro), [](auto r1) {}, [](auto r1) {});

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kReady);

  s->tryResume();

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kWait);

  s->tryResume();

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kWait);

  s->callback_queue.push(nullptr);

  BOOST_CHECK(s->getStatus() == Coroutine::state_type::kWait);

}

BOOST_AUTO_TEST_SUITE_END()
