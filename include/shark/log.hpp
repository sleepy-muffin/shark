/*
 * Project: shark-arts
 * Repository: https://github.com/leaked32/shark-arts
 *
 * File: shark/include/shark/log.hpp
 *
 * License: MIT
 */

#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <atomic>
#include <condition_variable>
#include <format>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "shark/common.hpp"

SHARK_BEGIN

namespace log
{
// inline std::mutex _logger_bus;

// Your message_type enum stays compatible
enum class message_type { INFO, DEBUG, EXCEPTION };

// Internal backend class
class backend
{
  public:
	backend() : worker_([this] { run(); }) {}

	~backend()
	{
		shutdown(); // automatic flush and stop
	}

	backend(const backend&) = delete;
	backend& operator=(const backend&) = delete;

	// Push a message into the queue
	void push(
		message_type type, std::string msg)
	{
		{
			std::lock_guard lock(mtx_);
			queue_.emplace(type, std::move(msg));
		}
		cv_.notify_one();
	}

	// Explicit shutdown (optional)
	void shutdown()
	{
		bool expected = false;
		if (!stopped_.compare_exchange_strong(expected, true))
			return; // already stopped

		{
			std::lock_guard lock(mtx_);
			stopping_ = true;
		}

		cv_.notify_all();

		if (worker_.joinable())
			worker_.join();
	}

  private:
	void run()
	{
		for (;;) {
			std::unique_lock lock(mtx_);
			cv_.wait(lock, [&] { return stopping_ || !queue_.empty(); });

			if (stopping_ && queue_.empty())
				break;

			auto [type, msg] = std::move(queue_.front());
			queue_.pop();

			lock.unlock();

			// Output exactly the message string
			std::cout << msg << std::endl;
		}
	}

  private:
	std::mutex mtx_;
	std::condition_variable cv_;
	std::queue<std::pair<message_type, std::string>> queue_;
	bool stopping_ = false;
	std::atomic_bool stopped_ = false;
	std::thread worker_;
};

// Function-local static instance ensures automatic creation
inline backend& instance()
{
	static backend b;
	return b;
}

// Submit API matches your previous API exactly
inline void submit(
	message_type type, const std::string& msg)
{
	instance().push(type, msg);
}

inline void submit(
	message_type type, std::string&& msg)
{
	instance().push(type, std::move(msg));
}

// Optional kill() function for explicit shutdown
inline void kill()
{
	instance().shutdown();
}

template <typename... Types>
constexpr void info(
	const std::format_string<Types...>& Fmt, Types&&... Args)
{
	std::string info_str = std::format(Fmt, std::forward<Types>(Args)...);
	submit(message_type::INFO, info_str);

	// std::string info_str = std::format("INFO:  {}", std::format(Fmt,
	// std::forward<Types>(Args)...)); submit(info_str);
}

template <typename... Types>
constexpr void debug(
	const std::format_string<Types...>& Fmt, Types&&... Args)
{
	// print<"INFO: {}">(std::format(Fmt, std::forward<Types>(Args)...));
	if constexpr (SHARK_DEBUG) {
		std::string info_str = std::format(Fmt, std::forward<Types>(Args)...);
		submit(message_type::DEBUG, info_str);
	}
}

template <typename... Types>
constexpr void exception(
	const std::format_string<Types...>& Fmt, Types&&... Args)
{
	std::string info_str = std::format(Fmt, std::forward<Types>(Args)...);
	submit(message_type::EXCEPTION, info_str);
}
} // namespace log

template <typename... Types>
constexpr void print(
	const std::format_string<Types...>& Fmt, Types&&... Args)
{
	std::string info_str = std::format(Fmt, std::forward<Types>(Args)...);
	log::submit(log::message_type::INFO, info_str);
}


struct pinch : std::exception
{
	template <typename... Types>
	constexpr pinch(
		const std::format_string<Types...>& Fmt, Types&&... Args)
	{
		std::string info_str = std::format(Fmt, std::forward<Types>(Args)...);
		msg_ = std::format("trace {}:{}", SHARK_FUNCTION, info_str);
	}
	// pinch(std::string_view func_name, std::string_view information) : std::exception{},
	// msg_(std::format("trace {}:{}", func_name, information)) { }
	
	pinch() : std::exception{}, msg_("{null}") {}
	
	virtual const char* what() const noexcept override
	{
		return msg_.c_str();
	}
	
	std::string msg_;
};

template <typename... Types>
void raise(
	const std::format_string<Types...>& Fmt, Types&&... Args)
{
	shark::print(Fmt, std::forward<Types>(Args)...);
	throw pinch(Fmt, std::forward<Types>(Args)...);
}

#undef assert
template <typename... Types>
void assert(
	bool condition, const std::format_string<Types...>& Fmt, Types&&... Args)
{
	if (!condition) {
		raise(Fmt, std::forward<Types>(Args)...);
	}
}

inline void assert(
	bool condition)
{
	assert(condition, "");
}

void indented_print(std::string_view s, char c = ' ', int n = 4);
std::string indent(std::string_view s, char c = ' ', int n = 4);


SHARK_END

