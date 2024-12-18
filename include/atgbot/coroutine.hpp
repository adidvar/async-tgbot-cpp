#pragma once

#include <atomic>
#include <coroutine>
#include <functional>

namespace ATgBot::Tools {
class Session;
}

namespace ATgBot {

class Coroutine {
 public:
  struct promise_type;

  Coroutine() : coro(nullptr) {}
  Coroutine(const Coroutine&) = delete; ///<don't copy it ever

  Coroutine(Coroutine&& other) noexcept {
    this->coro = other.coro;
    other.coro = handle_type(nullptr);
  }

  ~Coroutine() {
    if (coro)
      coro.destroy();
  }

  Coroutine& operator=(Coroutine&& other) noexcept {
    this->coro = other.coro;
    other.coro = handle_type(nullptr);
    return *this;
  }

  using handle_type = std::coroutine_handle<promise_type>;

  explicit Coroutine(handle_type h) : coro(h) {}

  struct promise_type {
    Coroutine get_return_object() {
      return Coroutine{handle_type::from_promise(*this)};
    }

    std::suspend_always initial_suspend() noexcept {
      m_state = State::kReady;
      return {};
    }
    std::suspend_always final_suspend() noexcept {
      if (m_state != State::kException)
        m_state = State::kDone;
      return {};
    }
    void unhandled_exception() {
      m_state = State::kException;
      m_exception = std::current_exception();
    }
    void return_void() {}

    enum class State {
      kNull,       // empty coroutine
      kReady,      // ready for execution
      kDone,       // done
      kException,  // got an exception
      kWait        // wait for some unknown event
    };

    State getState() {
      updateState();
      return m_state;
    }

    void pause(std::function<bool()>&& start_condition) {
      m_condition = std::move(start_condition);
      m_state = State::kWait;
    }
    void pause(std::function<bool()>&& start_condition, std::function<bool()>&& abort_condition) {
      m_condition = std::move(start_condition);
      m_abort_condition = std::move(abort_condition);
      m_state = State::kWait;
    }

    void updateState() {
      if (m_state == State::kWait) {
        //shouldnt happens
        if (!m_condition)
          throw std::runtime_error("a coro sleeps but m_condition is zero");
        if (m_condition && m_condition())
          m_state = State::kReady;
        if (m_abort_condition && m_abort_condition())
          m_state = State::kDone;
      }
    }
    //resume condition
    std::function<bool()> m_condition;
    //abort condition
    std::function<bool()> m_abort_condition;
    //state and exception processing
    std::exception_ptr m_exception;
    //current session
    ATgBot::Tools::Session* m_session = nullptr;

   private:
    State m_state;
  };
  friend class ATgBot::Tools::Session;

  using state_type = promise_type::State;

  state_type getState() const {
    if (!coro)
      return state_type::kNull;
    return coro.promise().getState();
  }

  bool tryResume() {
    bool ret_value = true;

    switch (getState()) {
      case state_type::kNull:
        return false;
      case state_type::kDone:
        return false;
      case state_type::kException:
        return false;
      case state_type::kReady:
        coro.resume();
        break;
      case state_type::kWait:
        return false;
    }
    if (getState() == state_type::kException)
      std::rethrow_exception(coro.promise().m_exception);

    return ret_value;
  }

 private:
  handle_type coro;
};

}  // namespace ATgBot

#include "atgbot/tools/session.hpp"
