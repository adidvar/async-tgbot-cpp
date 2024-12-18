#pragma once

#include <functional>
#include <queue>
#include <optional>
#include <mutex>

#include "eventfilter.hpp"

namespace ATgBot::Tools {

template <typename T>
class EventQueue {
 public:
  void setFilter(const EventFilter<T>& filter) {
    std::lock_guard _(m_mutex);
    m_has_changes = true;
    clear();
    m_filter = filter;
  }
  EventFilter<T> getFilter() const {
    std::lock_guard _(m_mutex);
    return m_filter;
  }
  void clear() {
    std::lock_guard _(m_mutex);
    while (!m_queue.empty())
      m_queue.pop();
  }
  bool empty() const {
    std::lock_guard _(m_mutex);
    return m_queue.empty();
  }

  void push(const T& element) {
    std::lock_guard _(m_mutex);
    if (m_filter.check(element))
      m_queue.emplace(element);
  }
  std::optional<T> pop() {
    std::lock_guard _(m_mutex);
    if (m_queue.empty())
      return std::nullopt;
    auto elem = m_queue.front();
    m_queue.pop();
    return elem;
  }
  void resetChanges() {
    std::lock_guard _(m_mutex);
    m_has_changes = false;
  }
  bool hasChanges() const {
    std::lock_guard _(m_mutex);
    return m_has_changes;
  }

 private:
  EventFilter<T> m_filter;
  mutable bool m_has_changes = true;
  std::queue<T> m_queue;
  mutable std::recursive_mutex m_mutex;
};

}  // namespace ATgBot
