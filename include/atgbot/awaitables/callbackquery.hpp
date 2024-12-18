#pragma once

#include "atgbot/coroutine.hpp"
#include "atgbot/tools/eventfilter.hpp"

namespace ATgBot::Awaitables {

class CBQueryAwaitable {
 public:
  CBQueryAwaitable(ATgBot::Tools::EventFilter<TgBot::CallbackQuery::Ptr> filter)
      : m_filter(filter) {}

  constexpr bool await_ready() const noexcept { return false; }

  void await_suspend(Coroutine::handle_type handle) noexcept {
    this->m_handle = handle;
    auto session = m_handle.promise().m_session;

    session->callback_queue.setFilter(m_filter);

    m_handle.promise().pause(
        [session]() { return !session->callback_queue.empty(); });
  }

  TgBot::CallbackQuery::Ptr await_resume() noexcept {
    auto e = m_handle.promise().m_session->callback_queue.pop();

    m_handle.promise().m_session->callback_queue.setFilter(
        Tools::EventFilter<TgBot::CallbackQuery::Ptr>{});

    return e.value();
  }

 private:
  Coroutine::handle_type m_handle;
  ATgBot::Tools::EventFilter<TgBot::CallbackQuery::Ptr> m_filter;
};

inline CBQueryAwaitable getCBQueryP(std::string prefix) {
  Tools::EventFilter<TgBot::CallbackQuery::Ptr> filter;
  filter.setEnabled(true);
  filter.setAdditionalFilter([prefix](TgBot::CallbackQuery::Ptr message) -> bool {
    return message->data.starts_with(prefix);
    });
  return CBQueryAwaitable(filter);
}
inline CBQueryAwaitable getCBQueryM(int64_t message_id) {
  Tools::EventFilter<TgBot::CallbackQuery::Ptr> filter;
  filter.setEnabled(true);
  filter.setAdditionalFilter([message_id](TgBot::CallbackQuery::Ptr message) -> bool {
    return message->message->messageId == message_id;
    });
  return CBQueryAwaitable(filter);
}
inline CBQueryAwaitable getCBQueryPM(std::string_view prefix, int64_t message_id) {
  Tools::EventFilter<TgBot::CallbackQuery::Ptr> filter;
  filter.setEnabled(true);
  filter.setAdditionalFilter([message_id,prefix](TgBot::CallbackQuery::Ptr message) -> bool {
    return message->message->messageId == message_id && message->data.starts_with(prefix);
    });
  return CBQueryAwaitable(filter);
}

}  
// namespace ATgBot::Awaitables
