#pragma once

#include <functional>

namespace ATgBot::Tools {

template <typename T>
class EventFilter {
  using Predicate = std::function<bool(T)>;

 public:
  bool check(const T& elem) const {
    if (!m_enabled)
      return false;
    if (m_additional_filter)
      return m_additional_filter(elem);
    return true;
  }
  void setAdditionalFilter(const Predicate& p) { m_additional_filter = p; }
  void setEnabled(bool v) { m_enabled = v; }

  bool m_enabled = false;
  Predicate m_additional_filter;
};


}  // namespace ATgBot
