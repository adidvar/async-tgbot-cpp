#pragma once

#include <functional>
#include <unordered_map>

#include "atgbot/tools/scheduler.hpp"
#include "atgbot/tools/session.hpp"

namespace ATgBot {

class AsyncBot {
 public:
  using MessageListener = std::function<Coroutine(TgBot::Message::Ptr)>;
  using InlineQueryListener = std::function<Coroutine(TgBot::InlineQuery::Ptr)>;
  using ChosenInlineResultListener =
      std::function<Coroutine(TgBot::ChosenInlineResult::Ptr)>;
  using CallbackQueryListener =
      std::function<Coroutine(TgBot::CallbackQuery::Ptr)>;
  using ShippingQueryListener =
      std::function<Coroutine(TgBot::ShippingQuery::Ptr)>;
  using PreCheckoutQueryListener =
      std::function<Coroutine(TgBot::PreCheckoutQuery::Ptr)>;
  using PollListener = std::function<Coroutine(TgBot::Poll::Ptr)>;
  using PollAnswerListener = std::function<Coroutine(TgBot::PollAnswer::Ptr)>;
  using ChatMemberUpdateListener =
      std::function<Coroutine(TgBot::ChatMemberUpdated::Ptr)>;
  using ChatJoinRequestListener =
      std::function<Coroutine(TgBot::ChatJoinRequest::Ptr)>;

  AsyncBot(TgBot::Bot& bot) : m_bot(bot) {
    m_bot.getEvents().onAnyMessage(
        std::bind(&AsyncBot::onMessage, this, std::placeholders::_1));
    m_bot.getEvents().onCallbackQuery(
        std::bind(&AsyncBot::onCallbackQuery, this, std::placeholders::_1));
    m_bot.getEvents().onEditedMessage(
        std::bind(&AsyncBot::onEditedMessage, this, std::placeholders::_1));
    m_bot.getEvents().onInlineQuery(
        std::bind(&AsyncBot::onInlineQuery, this, std::placeholders::_1));
    m_bot.getEvents().onChosenInlineResult(std::bind(
        &AsyncBot::onChosenInlineResult, this, std::placeholders::_1));
    m_bot.getEvents().onShippingQuery(
        std::bind(&AsyncBot::onShippingQuery, this, std::placeholders::_1));
    m_bot.getEvents().onPreCheckoutQuery(
        std::bind(&AsyncBot::onPreCheckoutQuery, this, std::placeholders::_1));
    m_bot.getEvents().onPoll(
        std::bind(&AsyncBot::onPoll, this, std::placeholders::_1));
    m_bot.getEvents().onPollAnswer(
        std::bind(&AsyncBot::onPollAnswer, this, std::placeholders::_1));
    m_bot.getEvents().onChatMember(
        std::bind(&AsyncBot::onChatMember, this, std::placeholders::_1));
    m_bot.getEvents().onChatJoinRequest(
        std::bind(&AsyncBot::onChatJoinRequest, this, std::placeholders::_1));
  }

  void run() {
    assert(!m_commands.empty());
    try {
      PLOGI << "Bot username: " << m_bot.getApi().getMe()->username.c_str();
      PLOGI << "Telegram bot longpoll started";
      TgBot::TgLongPoll longPoll(m_bot);
      while (true) {
        longPoll.start();
      }
    } catch (TgBot::TgException& e) {
      PLOGE << e.what();
      throw;
    }
  }

  void addCoro(Coroutine&& coro) { m_scheduler.pushCoro(std::move(coro)); }

  void addCommand(const std::string& command, MessageListener handler) {
    m_commands[command] = handler;
  }

  const TgBot::Api& getApi() const { return m_bot.getApi(); };

  void setMessageHandler(MessageListener handler) {
    m_message_handler = handler;
  }

  void setCallbackQueryHandler(CallbackQueryListener handler) {
    m_callback_handler = handler;
  }

  void setEditedMessageHandler(MessageListener handler) {
    m_edited_message_handler = handler;
  }

  void setInlineQueryHandler(InlineQueryListener handler) {
    m_inline_query_handler = handler;
  }

  void setChosenInlineResultHandler(ChosenInlineResultListener handler) {
    m_chosen_inline_result_handler = handler;
  }

  void setShippingQueryHandler(ShippingQueryListener handler) {
    m_shipping_query_handler = handler;
  }

  void setPreCheckoutQueryHandler(PreCheckoutQueryListener handler) {
    m_pre_checkout_query_handler = handler;
  }

  void setPollHandler(PollListener handler) { m_poll_handler = handler; }

  void setPollAnswerHandler(PollAnswerListener handler) {
    m_poll_answer_handler = handler;
  }

  void setChatMemberHandler(ChatMemberUpdateListener handler) {
    m_chat_member_handler = handler;
  }

  void setChatJoinRequestHandler(ChatJoinRequestListener handler) {
    m_chat_join_request_handler = handler;
  }

 private:

  void onMessage(const TgBot::Message::Ptr message) {
    PLOGD << "Bot received new message";
    m_scheduler.handleMessage(message);

    if (m_message_handler) {
      m_scheduler.pushCoro(m_message_handler(message));
    }

    if (!message->text.empty()) {
      PLOGD << "Bot received new command";
      for (auto command : m_commands)
        if (AsyncBot::checkCommand(command.first, message->text))
          m_scheduler.pushCoro(command.second(message));
    }
  }

  void onCallbackQuery(const TgBot::CallbackQuery::Ptr query) {
    PLOGD << "Bot received callback query";
    m_scheduler.handleCallbackQuery(query);
    if (m_callback_handler) {
      m_scheduler.pushCoro(m_callback_handler(query));
    }
  }

  void onEditedMessage(const TgBot::Message::Ptr message) {
    PLOGD << "Bot received edited message";
    m_scheduler.handleEditedMessage(message);
    if (m_edited_message_handler) {
      m_scheduler.pushCoro(m_edited_message_handler(message));
    }
  }

  void onInlineQuery(const TgBot::InlineQuery::Ptr query) {
    PLOGD << "Bot received inline query";
    m_scheduler.handleInlineQuery(query);
    if (m_inline_query_handler) {
      m_scheduler.pushCoro(m_inline_query_handler(query));
    }
  }

  void onChosenInlineResult(const TgBot::ChosenInlineResult::Ptr result) {
    PLOGD << "Bot received chosen inline result";
    m_scheduler.handleChosenInlineResult(result);
    if (m_chosen_inline_result_handler) {
      m_scheduler.pushCoro(m_chosen_inline_result_handler(result));
    }
  }

  void onShippingQuery(const TgBot::ShippingQuery::Ptr query) {
    PLOGD << "Bot received shipping query";
    m_scheduler.handleShippingQuery(query);
    if (m_shipping_query_handler) {
      m_scheduler.pushCoro(m_shipping_query_handler(query));
    }
  }

  void onPreCheckoutQuery(const TgBot::PreCheckoutQuery::Ptr query) {
    PLOGD << "Bot received pre-checkout query";
    m_scheduler.handlePreCheckoutQuery(query);
    if (m_pre_checkout_query_handler) {
      m_scheduler.pushCoro(m_pre_checkout_query_handler(query));
    }
  }

  void onPoll(const TgBot::Poll::Ptr poll) {
    PLOGD << "Bot received poll update";
    m_scheduler.handlePoll(poll);
    if (m_poll_handler) {
      m_scheduler.pushCoro(m_poll_handler(poll));
    }
  }

  void onPollAnswer(const TgBot::PollAnswer::Ptr answer) {
    PLOGD << "Bot received poll answer";
    m_scheduler.handlePollAnswer(answer);
    if (m_poll_answer_handler) {
      m_scheduler.pushCoro(m_poll_answer_handler(answer));
    }
  }

  void onChatMember(const TgBot::ChatMemberUpdated::Ptr update) {
    PLOGD << "Bot received chat member update";
    m_scheduler.handleChatMember(update);
    if (m_chat_member_handler) {
      m_scheduler.pushCoro(m_chat_member_handler(update));
    }
  }

  void onChatJoinRequest(const TgBot::ChatJoinRequest::Ptr request) {
    PLOGD << "Bot received chat join request";
    m_scheduler.handleChatJoinRequest(request);
    if (m_chat_join_request_handler) {
      m_scheduler.pushCoro(m_chat_join_request_handler(request));
    }
  }

 private:
  static bool checkCommand(std::string command, std::string text) {
    return text == command || text.starts_with(command + " ");
  };

  TgBot::Bot& m_bot;
  ATgBot::Tools::Scheduler m_scheduler;

  std::unordered_map<std::string, MessageListener> m_commands;
  MessageListener m_message_handler;
  CallbackQueryListener m_callback_handler;
  MessageListener m_edited_message_handler;
  InlineQueryListener m_inline_query_handler;
  ChosenInlineResultListener m_chosen_inline_result_handler;
  ShippingQueryListener m_shipping_query_handler;
  PreCheckoutQueryListener m_pre_checkout_query_handler;
  PollListener m_poll_handler;
  PollAnswerListener m_poll_answer_handler;
  ChatMemberUpdateListener m_chat_member_handler;
  ChatJoinRequestListener m_chat_join_request_handler;
};

};  // namespace ATgBot
