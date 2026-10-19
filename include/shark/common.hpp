/*
 * Project: shark
 * Repository: https://github.com/leaked32/shark
 *
 * File: shark/include/shark/common.hpp
 *
 * License: MIT
 */

#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <vector>

#define MMC_BEGIN namespace mmc {
#define MMC_END }

#define MMC_EXPORT

#if !defined(SHARK_FUNCTION)
#if defined(_MSC_VER)
#elif defined(__GNUC__) || defined(__clang__)
#define SHARK_FUNCTION __PRETTY_FUNCTION__
#endif
#endif

MMC_BEGIN
#ifdef __SHARK_DEBUG__
constexpr bool MMC_DEBUG = true;
#else
constexpr bool MMC_DEBUG = false;
#endif

template<class>
struct function_traits;

template<class TRes, class... TArgs>
struct function_traits<TRes(TArgs...)> {
  using return_type = TRes;
};

template<class TRes, class... TArgs>
struct function_traits<TRes (*)(TArgs...)> : function_traits<TRes(TArgs...)> {
};

template <typename F>
struct iter_args {
  template <class... Args>
  inline F& invoke() {
    return self();
  }

  template <typename T, typename... Args>
  inline F& invoke(T&& arg, Args&&... args) {
    self()(std::forward<T>(arg));
    return invoke(std::forward<Args>(args)...);
  }

  // template <typename... Args>
  // inline F& operator()(Args&&... args) {
  //   return self()(std::forward<Args>(args)...);
  // }
  //
  // template <typename T>
  // void operator()(const std::vector<T>& args) {
  //   for (const auto& arg : args) {
  //     self()(arg);
  //   }
  // }
private:
  inline F& self() {
    return *static_cast<F *>(this);
  }
};

MMC_END

