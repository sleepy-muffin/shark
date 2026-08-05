/*
 * Project: shark-arts
 * Repository: https://github.com/leaked32/shark-arts
 *
 * File: shark/include/shark/common.hpp
 *
 * License: MIT
 */

#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif

#define SHARK_BEGIN namespace shark {
#define SHARK_END }

#define SHARK_EXPORT

#if !defined(SHARK_FUNCTION)
#if defined(_MSC_VER)
#elif defined(__GNUC__) || defined(__clang__)
#define SHARK_FUNCTION __PRETTY_FUNCTION__
#endif
#endif

constexpr bool LIT_DEBUG = true;


template <class>
struct function_traits;

template <class TRes, class... TArgs>
struct function_traits<TRes(TArgs...)>
{
	using return_type = TRes;
};

template <class TRes, class... TArgs>
struct function_traits<TRes (*)(TArgs...)> : function_traits<TRes(TArgs...)>
{
};
