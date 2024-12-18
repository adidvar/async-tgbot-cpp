#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>

#include "eventfilter.hpp"

namespace ATgBot::Tools {
using DefaultTimer = std::chrono::high_resolution_clock;

class TimerEvent {
 public:
  TimerEvent() : time_point(DefaultTimer::now()) {}
  DefaultTimer::time_point time_point;
};

template <>
class EventFilter<TimerEvent> {
 public:
  bool check(const TimerEvent& elem) const {
    if (!m_enabled)
      return false;
    if (m_time_point.time_since_epoch().count() != 0)
      return DefaultTimer::now() >= m_time_point;
    return true;
  }
  void setTimePoint(const DefaultTimer::time_point& p) { m_time_point = p; }
  void setEnabled(bool v) { m_enabled = v; }

  bool m_enabled = false;
  DefaultTimer::time_point m_time_point;
};

class TimerEventGenerator {
 private:
  std::function<void()> func;
  std::atomic<bool> running;
  std::thread workerThread;

  void executePeriodically() {
    while (running) {
      func();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
  }

 public:
  explicit TimerEventGenerator(std::function<void()> f)
      : func(f), running(false) {}

  void start() {
    if (running) {
      return;
    }
    running = true;
    workerThread = std::thread(&TimerEventGenerator::executePeriodically, this);
  }

  void stop() {
    if (!running) {
      return;
    }
    running = false;
    if (workerThread.joinable()) {
      workerThread.join();
    }
  }

  ~TimerEventGenerator() { stop(); }
};

}  // namespace ATgBot::Tools
