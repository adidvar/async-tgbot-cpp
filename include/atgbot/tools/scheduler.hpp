#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#include "eventrouter.hpp"
#include "session.hpp"
#include "timerevent.hpp"

#include <tgbot/tgbot.h>

namespace ATgBot::Tools {

class Scheduler {
 public:
  using Task = std::shared_ptr<Session>;

  Scheduler(int thread_count = 4)
      : m_running(true),
        m_generator([this]() { handleTimerEvent(TimerEvent()); }) {
    m_generator.start();
    for (int i = 0; i < thread_count; ++i) {
      m_threads.emplace_back(&Scheduler::thread, this);
    }
  }

  ~Scheduler() {
    m_running = false;
    m_condition.notify_all();
    for (auto& thread : m_threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

  void pushCoro(Coroutine&& coro) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    m_sessions.push_back(Session::create(
        std::move(coro),
        std::bind(&Scheduler::addTaskToQueue, this, std::placeholders::_1),
        std::bind(&Scheduler::pushCoro, this, std::placeholders::_1)));
    addTaskToQueue(m_sessions.back());
  }

  void handleMessage(TgBot::Message::Ptr message) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    m_message_router.route(message);
  }

  void handleCallbackQuery(TgBot::CallbackQuery::Ptr query) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    m_callback_router.route(query);
  }

  void handleEditedMessage(TgBot::Message::Ptr message) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushEditedMessage(message);
      addTaskToQueue(session);
    }
  }

  void handleInlineQuery(TgBot::InlineQuery::Ptr query) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushInlineQuery(query);
      addTaskToQueue(session);
    }
  }

  void handleChosenInlineResult(TgBot::ChosenInlineResult::Ptr result) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushChosenInlineResult(result);
      addTaskToQueue(session);
    }
  }

  void handleShippingQuery(TgBot::ShippingQuery::Ptr query) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushShippingQuery(query);
      addTaskToQueue(session);
    }
  }

  void handlePreCheckoutQuery(TgBot::PreCheckoutQuery::Ptr query) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushPreCheckoutQuery(query);
      addTaskToQueue(session);
    }
  }

  void handlePoll(TgBot::Poll::Ptr poll) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushPoll(poll);
      addTaskToQueue(session);
    }
  }

  void handlePollAnswer(TgBot::PollAnswer::Ptr answer) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushPollAnswer(answer);
      addTaskToQueue(session);
    }
  }

  void handleMyChatMember(TgBot::ChatMemberUpdated::Ptr update) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushMyChatMember(update);
      addTaskToQueue(session);
    }
  }

  void handleChatMember(TgBot::ChatMemberUpdated::Ptr update) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushChatMember(update);
      addTaskToQueue(session);
    }
  }

  void handleChatJoinRequest(TgBot::ChatJoinRequest::Ptr request) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    for (auto& session : m_sessions) {
      //session->pushChatJoinRequest(request);
      addTaskToQueue(session);
    }
  }

  void handleTimerEvent(TimerEvent event) {
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    m_timer_router.route(event);
  }

 private:
  void addTaskToQueue(Task task) {
    std::lock_guard<std::recursive_mutex> lock(m_tasks_queue_mutex);
    if (std::find(m_tasks_queue.begin(), m_tasks_queue.end(), task) ==
        m_tasks_queue.end()) {
      m_tasks_queue.push_back(task);
    }

    m_condition.notify_one();
  }
  void updateTask(Task task) {
    m_message_router.update(task);
    m_callback_router.update(task);
    m_timer_router.update(task);
  }

  void thread() {
    while (m_running) {
      Task task_to_process = nullptr;
      {
        std::unique_lock<std::recursive_mutex> lock(m_tasks_queue_mutex);
        m_condition.wait(
            lock, [this]() { return !m_tasks_queue.empty() || !m_running; });

        if (!m_running)
          return;

        if (!m_tasks_queue.empty()) {
          task_to_process = m_tasks_queue.front();
          m_tasks_queue.erase(m_tasks_queue.begin());
        }
      }

      if (task_to_process) {
        processTask(task_to_process);
      }
    }
  }

  void processTask(Task task) {
    while (task->tryResume()) {}
    if (task->getStatus() == Coroutine::state_type::kNull) {
      removeSession(task);
      return;
    }
    if (task->getStatus() == Coroutine::state_type::kDone ||
        task->getStatus() == Coroutine::state_type::kException) {
      removeSession(task);
      return;
    }
    updateTask(task);
  }

  void removeSession(Task session) {
    m_message_router.remove(session);
    m_callback_router.remove(session);
    m_timer_router.remove(session);
    std::lock_guard<std::recursive_mutex> lock(m_sessions_mutex);
    m_sessions.erase(std::remove(m_sessions.begin(), m_sessions.end(), session),
                     m_sessions.end());
  }

 private:
  std::vector<Task> m_sessions;
  std::recursive_mutex m_sessions_mutex;

  std::vector<Task> m_tasks_queue;
  std::recursive_mutex m_tasks_queue_mutex;

  std::condition_variable_any m_condition;
  std::vector<std::thread> m_threads;

  std::atomic<bool> m_running;

  EventRouter<TgBot::Message::Ptr> m_message_router{&Session::message_queue};
  EventRouter<TgBot::CallbackQuery::Ptr> m_callback_router{
      &Session::callback_queue};
  EventRouter<TimerEvent> m_timer_router{&Session::timer_queue};

  TimerEventGenerator m_generator;
};

}  // namespace ATgBot::Tools
