#include "atgbot/tools/session.hpp"

namespace ATgBot::Tools {

void Session::execute() {
  add_to_queue_callback(shared_from_this());
}

void Session::pushCoro(Coroutine&& coro) const {
  add_new_coro_callback(std::move(coro));
}

//creates shared object
std::shared_ptr<Session> Session::create(Coroutine&& coro,
                                         QueueCallback q_callback,
                                         CoroCallback c_callback) {
  auto ptr = std::shared_ptr<Session>{new Session(std::move(coro))};
  ptr->coro.coro.promise().m_session = ptr.get();
  ptr->add_to_queue_callback = q_callback;
  ptr->add_new_coro_callback = c_callback;
  return ptr;
}

Coroutine::state_type Session::getStatus() const {
  std::lock_guard _(mutex);
  return coro.getState();
}

bool Session::tryResume() {
  std::lock_guard _(mutex);
  return coro.tryResume();
}

}  // namespace ATgBot::Tools
