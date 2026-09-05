/*
 * Project: shark
 * Repository: https://github.com/leaked32/shark
 *
 * File: shark/include/shark/str.hpp
 *
 * License: MIT
 */

#pragma once
#include <charconv>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <variant>

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "shark/common.hpp"

namespace shark::file {
class tlm_node {
	// Touhou Little Marks

public:
	void set_doc(std::string_view content) {
		doc = std::make_unique<std::string>(content);
	}

	bool has_doc() const {
		return doc != nullptr;
	}

	std::optional<std::string_view> get_doc() const {
		return doc != nullptr ? std::optional<std::string_view>{*doc} : std::optional<std::string_view>{};
	}

	// std::string_view doesn't gaurantee null-terminate
	tlm_node& get_child(std::string_view key) {
		if (children == nullptr) {
			children = std::make_unique<decltype(children)::element_type>();
		}

		auto p = children->try_emplace(std::string(key));
		return p.first->second;
	}

	template<typename Ty>
	tlm_node& operator[](Ty&& key) {
		return get_child(std::forward<Ty>(key));
	}

	std::string print(unsigned short indent = 0) const;

	std::optional<std::reference_wrapper<const std::map<std::string, tlm_node>> > c_get_children() const {
		if (children) {
			return *children;
		}
		return {};
	}

private:
	// lit::print("{}  {}", sizeof (std::map<std::string, tlm_node>), sizeof
	// (std::unique_ptr<std::map<std::string, tlm_node>>));
	std::unique_ptr<std::map<std::string, tlm_node> > children = nullptr;
	std::unique_ptr<std::string> doc = nullptr;
};

std::string read(std::string_view path);
std::vector<char> read_binary(const std::string& filename);
tlm_node tlm_node_read(std::string_view path);
//
}; // namespace shark::file

namespace shark::str {
std::string replace(std::string_view input, std::string_view from, std::string_view to);

inline std::vector<std::string> split(const std::string& text) {
	std::vector<std::string> result;

	size_t i = 0;

	while (i < text.size()) {
		// Skip whitespace
		while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) {
			++i;
		}

		if (i >= text.size())
			break;

		size_t start = i;

		// Read token
		while (i < text.size() && !std::isspace(static_cast<unsigned char>(text[i]))) {
			++i;
		}

		result.emplace_back(text.substr(start, i - start));
	}

	return result;
}

inline std::vector<std::string> split(const std::string& text, char delimiter) {
	std::vector<std::string> result;

	size_t start = 0;

	while (true) {
		size_t pos = text.find(delimiter, start);

		if (pos == std::string::npos) {
			result.emplace_back(text.substr(start));
			break;
		}

		result.emplace_back(text.substr(start, pos - start));
		start = pos + 1;
	}

	return result;
}

std::optional<std::string> trim(const std::string& s);
unsigned short leading_space_count(const std::string& s);
bool is_empty_or_whitespace(const std::string& s);
void remove_whitespace(std::string& s);
size_t utf8_codepoints(std::string_view s);
size_t estimate_tokens(std::string_view s);
std::string to_lower(const std::string& s);

template<typename T>
std::tuple<T, T> from_interval_text(std::string_view interval) {
	std::string intr = str::replace(interval, " ", "");
}

void forcibly_print_string(const std::string& input);
void forcibly_print_vector(const std::vector<std::string>& input);

void indented_print(std::string_view s, char c = ' ', int n = 4);
std::string indent(std::string_view s, char c = ' ', int n = 4);

} // namespace shark::str



namespace shark::toml {
class Toml {
public:
	using Table = std::map<std::string, Toml>;
	using Value = std::variant<std::monostate, bool, int64_t, double, std::string, Table>;

	Toml() = default;
	explicit Toml(Value value) : value_(std::move(value)) {}

	bool is_table() const;
	bool is_string() const;
	bool is_integer() const;
	bool is_real() const;
	bool is_boolean() const;

	const Toml& at(const std::string& key) const;
	Toml& at(const std::string& key);
	const Toml* find(const std::string& key) const;
	Toml* find(const std::string& key);

	const Table& table() const;
	Table& table();
	const std::string& string() const;
	int64_t integer() const;
	double real() const;
	bool boolean() const;

private:
	Value value_;
};

Toml parse(std::string_view text);
Toml parse_toml(const std::filesystem::path& path);
}

namespace shark::json {
class Json {
public:
	using Array = std::vector<Json>;
	using Object = std::map<std::string, Json>;
	using Value = std::variant<std::nullptr_t, bool, double, std::string, Array, Object>;

	Json() :
		value_(nullptr) {
	}

	explicit Json(Value value) :
		value_(std::move(value)) {
	}

	static Json parse(std::string_view text);
	bool is_null() const;
	bool is_string() const;
	const std::string& string() const;
	double number() const;
	const Array& array() const;
	const Object& object() const;
	const Json& at(const std::string& key) const;
	const Json* find(const std::string& key) const;

private:
	Value value_;
};
} // namespace shark
