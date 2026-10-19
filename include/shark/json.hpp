#pragma once

#include <vector>
#include <string>
#include <map>
#include <variant>

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

  [[nodiscard]] static Json loads(std::string_view path);
  [[nodiscard]] std::string dumps(int indentation = 0) const;
  [[nodiscard]] static Json parse(std::string_view text);

  [[nodiscard]] bool is_null() const;
  [[nodiscard]] bool is_string() const;
  [[nodiscard]] const std::string& string() const;
  [[nodiscard]] double number() const;
  [[nodiscard]] const Array& array() const;
  [[nodiscard]] const Object& object() const;
  [[nodiscard]] const Json& at(const std::string& key) const;
  [[nodiscard]] const Json* find(const std::string& key) const;

private:
  Value value_;
};

}
