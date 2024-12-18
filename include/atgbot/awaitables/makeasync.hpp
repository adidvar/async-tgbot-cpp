/**
 * @file make_async.hpp
 * @brief Provides functionality to execute functions asynchronously using coroutines.
 *
 * This file defines the `makeAsync` class template that enables asynchronous execution 
 * of callable objects (functions or functors) in a separate thread using C++ coroutines.
 * It supports both functions with and without return values.
 *
 */

#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>

#include "atgbot/coroutine.hpp"

namespace ATgBot::Awaitables {

/**
 * @brief A class that executes a function asynchronously using a separate thread and coroutines.
 *
 * This class supports asynchronous execution for callable objects with or without return values.
 * It allows a coroutine to suspend while the function executes and resumes once the execution is complete.
 *
 * @tparam T The type of the callable object (function or functor).
 * @tparam Args The types of arguments passed to the callable object.
 */
template <typename T, typename... Args>
class makeAsync {};

/**
 * @brief Specialization of `makeAsync` for functions or functors with return values.
 *
 * This specialization supports asynchronous execution for callable objects that return a result.
 * The result is stored and returned when the coroutine resumes.
 *
 * @tparam T The type of the callable object.
 * @tparam Args The types of arguments passed to the callable object.
 */
template <typename T, typename... Args>
  requires std::invocable<T, Args...> &&
           (!std::is_same_v<void, std::invoke_result_t<T, Args...>>)
struct makeAsync<T, Args...> {
  using R =
      std::invoke_result_t<T,
                           Args...>;  ///< Result type of the invoked callable.

  template <typename T, typename Tuple, std::size_t... Indices>
  static auto invoke_with_indices(T&& m_object, Tuple&& m_args,
                                  std::index_sequence<Indices...>) {
    return std::invoke(std::forward<T>(m_object),
                       std::forward<decltype(std::get<Indices>(m_args))>(
                           std::get<Indices>(m_args))...);
  }

 public:
  /**
   * @brief Construct a new `makeAsync` object.
   *
   * @param object The callable object (function or functor).
   * @param args The arguments to pass to the callable.
   */
  makeAsync(T&& object, Args&&... args)
      : m_object(std::forward<T>(object)),
        m_args(std::forward<Args>(args)...) {}

  /**
   * @brief Check if the coroutine is ready to execute.
   *
   * The coroutine is never ready immediately and will always suspend.
   *
   * @return true if the coroutine is ready (always `false` for this implementation).
   */
  bool await_ready() const noexcept { return false; }

  /**
   * @brief Suspend the coroutine and execute the function asynchronously in a separate thread.
   *
   * This method is invoked when the coroutine is suspended. It creates a new thread to invoke
   * the callable object with the provided arguments and stores the result.
   *
   * @param handle The coroutine handle.
   */
  void await_suspend(Coroutine::handle_type handle) noexcept {
    this->m_handle = handle;

    // Create a new thread to execute the callable.
    m_thread = std::make_unique<std::thread>([this]() {
      // Invoke the callable with the arguments.
      R result =
          makeAsync::invoke_with_indices(std::forward<T>(m_object), m_args,
                                         std::index_sequence_for<Args...>{});

      std::lock_guard _(m_mutex);
      m_result =
          std::move(result);  // Move the result to avoid unnecessary copies.
      m_handle.promise().m_session->execute();
    });

    m_handle.promise().pause([this]() {
      std::lock_guard _(m_mutex);
      return m_result.has_value();  ///< Resume when the result is available.
    });
  }

  /**
   * @brief Resume the coroutine once the result of the async task is available.
   *
   * This method is invoked when the coroutine is resumed. It waits for the result to be available
   * and returns it.
   *
   * @return The result of the callable execution.
   */
  R await_resume() noexcept {
    if (!m_result.has_value()) {
      PLOGW << "Resume of async";
    }
    m_thread->join();         ///< Wait for the thread to finish execution.
    return m_result.value();  ///< Return the result of the execution.
  }

 private:
  Coroutine::handle_type m_handle;  ///< The coroutine handle.
  T m_object;                  ///< The callable object (function or functor).
  std::tuple<Args...> m_args;  ///< The arguments for the callable.
  std::optional<R> m_result;   ///< The result of the callable execution.
  std::unique_ptr<std::thread> m_thread{
      nullptr};        ///< The thread in which the callable is executed.
  std::mutex m_mutex;  ///< Mutex for thread synchronization.
};

/**
 * @brief Specialization of `makeAsync` for functions or functors without return values.
 *
 * This specialization supports asynchronous execution for callable objects that do not return a result.
 *
 * @tparam T The type of the callable object.
 * @tparam Args The types of arguments passed to the callable object.
 */
template <typename T, typename... Args>
  requires std::invocable<T, Args...> &&
           std::is_same_v<void, std::invoke_result_t<T, Args...>>
struct makeAsync<T, Args...> {

  template <typename T, typename Tuple, std::size_t... Indices>
  static auto invoke_with_indices(T&& m_object, Tuple&& m_args,
                                  std::index_sequence<Indices...>) {
    return std::invoke(std::forward<T>(m_object),
                       std::forward<decltype(std::get<Indices>(m_args))>(
                           std::get<Indices>(m_args))...);
  }

 public:
  /**
   * @brief Construct a new `makeAsync` object.
   *
   * @param object The callable object (function or functor).
   * @param args The arguments to pass to the callable.
   */
  makeAsync(T&& object, Args&&... args)
      : m_object(std::forward<T>(object)),
        m_args(std::forward<Args>(args)...) {}

  /**
   * @brief Check if the coroutine is ready to execute.
   *
   * The coroutine is never ready immediately and will always suspend.
   *
   * @return true if the coroutine is ready (always `false` for this implementation).
   */
  bool await_ready() const noexcept { return false; }

  /**
   * @brief Suspend the coroutine and execute the function asynchronously in a separate thread.
   *
   * This method is invoked when the coroutine is suspended. It creates a new thread to invoke
   * the callable object with the provided arguments.
   *
   * @param handle The coroutine handle.
   */
  void await_suspend(Coroutine::handle_type handle) noexcept {
    this->m_handle = handle;

    // Create a new thread to execute the callable.
    m_thread = std::make_unique<std::thread>([this]() {
      // Invoke the callable with the arguments.
      makeAsync::invoke_with_indices(std::forward<T>(m_object), m_args,
                                     std::index_sequence_for<Args...>{});

      m_ready = true;
      m_handle.promise().m_session->execute();
    });

    // Define the condition for resuming the coroutine.
    m_handle.promise().pause([this]() {
      return m_ready.load();  ///< Resume when the result is available.
    });
  }

  /**
   * @brief Resume the coroutine once the async task is complete.
   *
   * This method is invoked when the coroutine is resumed. It waits for the callable execution to finish.
   */
  void await_resume() noexcept {
    m_thread->join();  ///< Wait for the thread to finish execution.
  }

 private:
  Coroutine::handle_type m_handle;  ///< The coroutine handle.
  T m_object;                  ///< The callable object (function or functor).
  std::tuple<Args...> m_args;  ///< The arguments for the callable.
  std::atomic<bool> m_ready{false};
  std::unique_ptr<std::thread> m_thread{
      nullptr};  ///< The thread in which the callable is executed.
};

/**
 * @brief Deduction guide for the `makeAsync` template.
 *
 * Allows deduction of template arguments when constructing `makeAsync` objects.
 *
 * @tparam T The type of the callable object.
 * @tparam Args The types of arguments passed to the callable object.
 */
template <typename T, typename... Args>
makeAsync(T, Args...) -> makeAsync<T, Args...>;

}  // namespace ATgBot::Awaitables
