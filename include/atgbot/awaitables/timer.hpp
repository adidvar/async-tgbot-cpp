#pragma once

#include "atgbot/coroutine.hpp"
#include "atgbot/tools/timerevent.hpp"

#include <chrono>

namespace ATgBot::Awaitables {

class TimerAwaitable {
 public:
  using Clock = ATgBot::Tools::DefaultTimer;
  using Duration = Clock::duration;
  using TimePoint = Clock::time_point;

  TimerAwaitable(TimePoint until)
      : m_filter{.m_enabled = true, .m_time_point = until} {}

  bool await_ready() const noexcept {
    return m_filter.check(ATgBot::Tools::TimerEvent());
  }

  void await_suspend(Coroutine::handle_type handle) noexcept {
    this->m_handle = handle;
    auto session = m_handle.promise().m_session;

    session->timer_queue.setFilter(m_filter);

    m_handle.promise().pause(
        [session]() { return !session->timer_queue.empty(); });
  }

  void await_resume() noexcept {
    auto e = m_handle.promise().m_session->timer_queue.pop();

    m_handle.promise().m_session->timer_queue.setFilter(
        Tools::EventFilter<Tools::TimerEvent>{});
  }

 private:
  Coroutine::handle_type m_handle;
  ATgBot::Tools::EventFilter<ATgBot::Tools::TimerEvent> m_filter;
};

template <class T, class U>
TimerAwaitable waitFor(std::chrono::duration<T,U> duration) {
  auto time_point = ATgBot::Tools::DefaultTimer::now() + std::chrono::duration_cast<ATgBot::Tools::DefaultTimer::duration>(duration); 
  return TimerAwaitable(time_point);
}

template <class T, class U>
TimerAwaitable waitUntil(std::chrono::time_point<T,U> time_point) {
  return TimerAwaitable(std::chrono::time_point_cast<ATgBot::Tools::DefaultTimer::time_point>(time_point));
}

}  // namespace ATgBot::Awaitables
