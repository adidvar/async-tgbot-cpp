#pragma once

#include <algorithm>
#include <queue>

#include "tgbot/tgbot.h"

#include "atgbot/coroutine.hpp"
#include "atgbot/tools/eventqueue.hpp"
#include "atgbot/tools/timerevent.hpp"

namespace ATgBot {
class Coroutine;
}

namespace ATgBot::Tools {

  using ATgBot::Coroutine;

class Session : public std::enable_shared_from_this<Session> {

  using QueueCallback = std::function<void(std::shared_ptr<Session>)>;
  using CoroCallback = std::function<void(Coroutine&&)>;

  //private constructor
  Session(Coroutine&& coro) : coro(std::move(coro)) {}

 public:
  //creates shared object
  static std::shared_ptr<Session> create(Coroutine&& coro,
                                         QueueCallback q_callback,
                                         CoroCallback c_callback);
  //returns status
  Coroutine::state_type getStatus() const;
  // trying to resume
  bool tryResume();
  //for awaitables only
  void execute();
  void pushCoro(Coroutine&& coro) const;

  // messages processing
  //std::Queue<TimerEvent> timer_queue;
  EventQueue<TgBot::Message::Ptr> message_queue;
  EventQueue<TgBot::CallbackQuery::Ptr> callback_queue;
  EventQueue<TimerEvent> timer_queue;
 private:
  // mutex
  mutable std::recursive_mutex mutex;
  // state
  Coroutine coro;
  // scheduler callbacks
  QueueCallback add_to_queue_callback;
  CoroCallback add_new_coro_callback;

  friend class Scheduler;
  friend class SessionPrivate;
};

}  // namespace ATgBot
