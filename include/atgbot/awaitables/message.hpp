#pragma once

#include "atgbot/coroutine.hpp"

namespace ATgBot::Awaitables {

class MessageAwaitable {
 public:
  MessageAwaitable(ATgBot::Tools::EventFilter<TgBot::Message::Ptr> filter)
      : m_filter(filter) {}

  constexpr bool await_ready() const noexcept { return false; }

  void await_suspend(Coroutine::handle_type handle) noexcept {
    this->m_handle = handle;
    auto session = m_handle.promise().m_session;

    session->message_queue.setFilter(m_filter);

    m_handle.promise().pause(
        [session]() { return !session->message_queue.empty(); });
  }

  TgBot::Message::Ptr await_resume() noexcept {
    auto e = m_handle.promise().m_session->message_queue.pop();

    m_handle.promise().m_session->callback_queue.setFilter(
        Tools::EventFilter<TgBot::CallbackQuery::Ptr>{});

    return e.value();
  }

 private:
  Coroutine::handle_type m_handle;
  ATgBot::Tools::EventFilter<TgBot::Message::Ptr> m_filter;
};

inline MessageAwaitable getMessageU(int64_t user_id) {
  ATgBot::Tools::EventFilter<TgBot::Message::Ptr> filter;
  filter.setEnabled(true);
  filter.setAdditionalFilter([user_id](TgBot::Message::Ptr message) {
    return message->from->id == user_id;
  });
  return MessageAwaitable(filter);
}
inline MessageAwaitable getMessageG(int64_t group_id) {
  ATgBot::Tools::EventFilter<TgBot::Message::Ptr> filter;
  filter.setEnabled(true);
  filter.setAdditionalFilter([group_id](TgBot::Message::Ptr message) {
    return message->chat->id == group_id;
  });
  return MessageAwaitable(filter);
}
inline MessageAwaitable getMessageUG(int64_t user_id, int64_t group_id) {
  ATgBot::Tools::EventFilter<TgBot::Message::Ptr> filter;
  filter.setEnabled(true);
  filter.setAdditionalFilter([user_id, group_id](TgBot::Message::Ptr message) {
    return message->from->id == user_id && message->chat->id == group_id;
  });
  return MessageAwaitable(filter);
}
}  // namespace ATgBot::Awaitables
