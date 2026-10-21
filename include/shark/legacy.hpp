#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include "shark/common.hpp"
#include "shark/log.hpp"

namespace shark {
// For specified error code
template <auto F, auto N, typename... Types>
auto checked_invoke(
    Types&&... Args)
{
  auto result = F(std::forward<Types>(Args)...);
  if (decltype(N)(result) != N) {
    log::debug("Invocation of function has failed: {} {:#x}", SHARK_FUNCTION,
               decltype(N)(result));
    raise("Invocation of function has failed");
  }
  return result;
}

template <auto F, typename... Types>
auto yinvoke(
    Types&&... Args)
{
  return checked_invoke<F, 0UL>(std::forward<Types>(Args)...);
}

template <auto F, typename... Types>
auto einvoke(
    Types&&... Args)
{
  return checked_invoke<F, 0UL>(std::forward<Types>(Args)...);
}

template <typename T, T good>
struct rcheck
{
  void check(
      T v) const
  {
    if (v != good) {
      log::debug("Invocation of function has failed: {} {:#x}", SHARK_FUNCTION,
                 static_cast<unsigned long>(v));
      throw pinch{"Invocation of function has failed"};
    }
  }
  template <typename... Types>
  void operator=(
      Types&&... values) const
  {
    check(std::forward<Types>(values)...);
  }
};

std::string find_system_font(std::string_view name);
}
