#pragma once

#include "atgbot/coroutine.hpp"

namespace ATgBot::Awaitables {

class createCoro {
 public:
  explicit createCoro(Coroutine&& coro) : m_coro(std::move(coro)) {}

  bool await_ready() const noexcept { return false; }

  void await_suspend(Coroutine::handle_type handle) noexcept {
    handle.promise().m_session->pushCoro(std::move(m_coro));
  }

  void await_resume() noexcept {}

 private:
  Coroutine m_coro;
};

}  // namespace ATgBot::Awaitables
