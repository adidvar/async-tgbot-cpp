#pragma once

#include "eventqueue.hpp"
#include "session.hpp"

namespace ATgBot::Tools {

/**
 * @brief EventRouter is a template class that routes events to registered sessions.
 * 
 * @tparam T The type of events to be routed.
 */
template <typename T>
class EventRouter {
 public:
  EventRouter() = delete;
  /**
   * @brief Constructor that initializes the EventRouter with a member pointer to the EventQueue.
   * 
   * @param p Pointer to the EventQueue member within Session.
   */
  explicit EventRouter(EventQueue<T> Session::*p) : m_pointer(p) {}

  /**
   * @brief Removes a session from the list of managed sessions.
   * 
   * @param session Shared pointer to the session to remove.
   */
  void remove(std::shared_ptr<Session> session) {
    std::lock_guard _(m_mutex);
    m_sessions.erase(std::remove(m_sessions.begin(), m_sessions.end(), session),
                     m_sessions.end());
  }

  /**
   * @brief Updates a session by removing it and re-adding it if it has changes.
   * 
   * @param session Shared pointer to the session to update.
   */
  void update(std::shared_ptr<Session> session) {

    EventQueue<T>& queue = (*session.get()).*m_pointer;
    if (!queue.hasChanges())
      return;

    std::lock_guard _(m_mutex);
    remove(session);

    EventFilter<T> filter = queue.getFilter();
    if (filter.m_enabled) {
      m_sessions.push_back(session);
    }
  }

  /**
   * @brief Routes a message to all registered sessions.
   * 
   * @param message The message to route.
   */
  void route(const T& message) {
    std::lock_guard _(m_mutex);
    for (auto& session : m_sessions) {
      ((*session.get()).*m_pointer).push(message);
      session->execute();
    }
  }

 private:
  std::vector<std::shared_ptr<Session>>
      m_sessions;  ///< List of managed sessions.
  EventQueue<T> Session::*
      m_pointer;  ///< Pointer to the EventQueue member in Session.
  std::recursive_mutex m_mutex;
};

}  // namespace ATgBot::Tools
