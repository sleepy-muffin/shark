//
// Created by menv on 9/4/26.
//

#ifndef SHARK_JSON_HPP
#define SHARK_JSON_HPP

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace shark {
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

#endif // SHARK_JSON_HPP
