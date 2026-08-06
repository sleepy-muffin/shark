/*
 * Project: shark-arts
 * Repository: https://github.com/leaked32/shark-arts
 *
 * File: shark/include/shark/legacy.hpp
 *
 * License: MIT
 */

#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include "shark/common.hpp"
#include "shark/log.hpp"

SHARK_BEGIN

// For specified error code
template <auto F, auto N, typename... Types>
auto checked_invoke(
	Types&&... Args)
{
	auto result = F(std::forward<Types>(Args)...);
	if (decltype(N)(result) != N) {
		log::debug("Invocation of function has failed: {} {:#x}", SHARK_FUNCTION,
				   decltype(N)(result));
		throw pinch{"Invocation of function has failed"};
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

struct profiler
{
	using clock = std::chrono::steady_clock;
	using time_point = clock::time_point;
	using duration = std::chrono::duration<double>;

	profiler(
		size_t reserve, bool print_on_lap) : start(clock::now()), print_on_lap(print_on_lap)
	{
		laps.reserve(reserve);
	}

	void lap(
		std::optional<std::string_view> msg = std::nullopt)
	{
		time_point end = clock::now();
		const duration diff = end - start;
		laps.emplace_back(diff);

		if (print_on_lap) {
			print_duration(diff, msg);
		}

		reset(); // exclude emplace_back
	}

	void reset()
	{
		start = clock::now();
	}

	static void print_duration(
		duration value, std::optional<std::string_view> msg = std::nullopt)
	{
		const auto milliseconds = std::chrono::duration<double, std::milli>(value).count();

		if (msg.has_value()) {
			log::info("{}: {:.6f} ms", msg.value(), milliseconds);
		}
		else {
			log::info("{:.6f} ms", milliseconds);
		}
	}

	time_point start;
	bool print_on_lap;
	std::vector<duration> laps;
};

std::string find_system_font(std::string_view name);

SHARK_END
