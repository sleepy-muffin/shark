/*
 * Project: shark
 * Repository: https://github.com/leaked32/shark
 *
 * File: shark/src/str.cpp
 *
 * License: MIT
 */

#include "shark/str.hpp"

#include <cmath>

#include "shark/log.hpp"

#include <fstream>
#include <sstream>

std::string shark::str::replace(
	std::string_view input, std::string_view from, std::string_view to)
{
	using T = decltype(input);

	const size_t len_from = from.size();
	const size_t len_to = to.size();
	const size_t len_input = input.size();

	size_t pre_count = 0;
	for (size_t occur = 0; occur = input.find(from, occur), occur != T::npos;) {
		occur += len_from;
		pre_count += 1ul;
	}

	if (pre_count == 0) {
		throw pinch("expression not fonud");
	}

	size_t output_capacity = len_input + (len_to - len_from) * pre_count;
	std::string output;
	output.resize(output_capacity);

	// std::println("{} {} {} {}", input, from, to, pre_count);

	size_t it_last = 0;
	size_t it_count = 0;
	for (size_t it = 0; it = input.find(from, it), it != T::npos;) {
		// std::println("{} ", output);

		// 1. Copy the buffer before the found substring to the output, the
		// begin of the buffer should be last time, last_it = it + len_from
		std::copy(input.begin() + it_last, input.begin() + it,
				  output.begin() + it_last + it_count * (len_to - len_from));
		// 2. Copy the new substring to the output
		std::copy(to.begin(), to.end(), output.begin() + it + it_count * (len_to - len_from));

		it += len_from;
		it_last = it;

		it_count += 1;
	}
	// std::println("{} {}", output.size(), output.length(), it_last + it_count
	// * (len_to - len_from)); size() does not count the ending \0 by
	// std::string, but \0 I put is counted, so size() and length() can't be
	// used.
	// 3. Copy the tail buffer
	std::copy(input.begin() + it_last, input.end(),
			  output.begin() + it_last + it_count * (len_to - len_from));

	return output;
}

bool shark::str::is_empty_or_whitespace(
	const std::string& s)
{
	return std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });
}

std::optional<std::string> shark::str::trim(
	const std::string& s)
{
	if (std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); })) {
		return {};
	}

	auto start =
		std::find_if_not(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); });

	auto end =
		std::find_if_not(s.rbegin(), s.rend(), [](unsigned char c) { return std::isspace(c); })
			.base();

	if (start >= end)
		return "";
	return std::string(start, end);
}

unsigned short shark::str::leading_space_count(
	const std::string& s)
{
	unsigned short count = 0;
	auto start =
		std::find_if_not(s.begin(), s.end(), [&count](unsigned char c) { return std::isspace(c); });

	return count;
}

void shark::str::remove_whitespace(
	std::string& s)
{
	s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }),
			s.end());
}

std::string shark::file::read(
	std::string_view path)
{
	// Warning std::string_view is not null-terminated guaranteed.
	std::ifstream file(std::string(path), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		shark::raise("Cannot open file: {}", path);
	}

	std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return str;
}

std::vector<char> shark::file::read_binary(
	const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + filename);
	size_t size = file.tellg();
	std::vector<char> buffer(size);
	file.seekg(0);
	file.read(buffer.data(), size);
	return buffer;
}

shark::file::tlm_node shark::file::tlm_node_read(
	std::string_view file_path)
{
	const std::string identifier = "=^..^= ";
	std::string ctx = shark::file::read(file_path);

	decltype(ctx)::iterator ctx_begin = ctx.begin();

	using ty_indexes = std::vector<std::string>;
	ty_indexes indexes;

	tlm_node node;
	auto mk_doc = [](tlm_node& node, std::string_view doc, const ty_indexes& indexes)
	{
		tlm_node* idx = &node;
		for (const auto& index : indexes) {
			idx = &(*idx)[index];
		}
		idx->set_doc(doc);
	};

	auto decribe_id = [](std::string_view id, ty_indexes& indexes)
	{
		short inherit_from = -1;

		std::string_view::iterator id_begin = id.end();
		for (auto id_i = id.begin(); id_i != id.end(); ++id_i) {
			if (std::isspace(static_cast<unsigned char>(*id_i))) {
				continue;
			}
			if (inherit_from == -1) {
				if (*id_i == ':') {
					inherit_from = 1;
					continue;
				}
				else {
					inherit_from = 0;
					indexes.clear();
				}
			}
			if (id_begin == id.end()) {
				id_begin = id_i;
			}

			if (*id_i == ':') {
				indexes.emplace_back(id_begin, id_i);
				id_begin = id_i + 1;
			}
		}
	};

	for (size_t pos = 0; pos < ctx.size(); ++pos) {
		if (pos == 0 ? true : ctx.at(pos - 1) == '\n') {
			if (std::string_view(ctx.data() + pos, identifier.size()).compare(identifier) == 0) {
				// line of the key
				if (!indexes.empty()) {
					auto tmp_begin = ctx_begin + 1;
					auto tmp_end = ctx.begin() + pos - 1;
					mk_doc(node, tmp_end > tmp_begin ? std::string_view{tmp_begin, tmp_end} : "",
						   indexes);
				}

				size_t pos_next = ctx.find_first_of('\n', pos); // Move to the next new line
				size_t ps_id_begin = pos + identifier.size();

				std::string_view id(ctx.data() + ps_id_begin, pos_next - ps_id_begin);
				decribe_id(id, indexes);

				// lit::log::info("id:{}", id);

				ctx_begin = ctx.begin() + pos_next;
			}
			// Not identifier line, don't care about it.'
		}
	}

	// node.print();
	return node;
}

size_t shark::str::utf8_codepoints(
	std::string_view s)
{
	size_t count = 0;

	for (unsigned char c : s) {
		if ((c & 0xC0) != 0x80)
			++count;
	}

	return count;
}

std::string shark::str::to_lower(
	const std::string& s)
{
	std::string result = s;
	std::transform(result.begin(), result.end(), result.begin(),
				   [](unsigned char c) { return std::tolower(c); });
	return result;
}

size_t shark::str::estimate_tokens(
	std::string_view s)
{
	size_t ascii = 0;
	size_t non_ascii_chars = 0;

	for (unsigned char c : s) {
		if (c < 128)
			++ascii;
		else if ((c & 0xC0) != 0x80)
			++non_ascii_chars;
	}

	return (ascii + 3) / 4 + non_ascii_chars;
}

std::string shark::file::tlm_node::print(
	unsigned short indent) const
{
	std::ostringstream ss;
	std::string indent_str(indent, ' ');

	// --- children ---
	if (children) {
		for (const auto& [key, child] : *children) {
			ss << indent_str << "child:" << key << ":\n";
			ss << child.print(indent + 2);
		}
	}

	// --- doc (word wrapped, aligned) ---
	if (doc) {
		std::string_view text = *doc;

		std::string prefix = indent_str + "doc: ";
		std::string continuation(prefix.size(), ' ');

		bool first_line_global = true;

		size_t start = 0;
		while (start <= text.size()) {
			size_t end = text.find('\n', start);
			if (end == std::string_view::npos)
				end = text.size();

			std::string_view line = text.substr(start, end - start);

			// --- empty line (preserve it) ---
			if (line.empty()) {
				if (first_line_global) {
					ss << prefix << '\n';
					first_line_global = false;
				}
				else {
					ss << continuation << '\n';
				}
			}
			else {
				// --- word wrap THIS line only ---
				size_t pos = 0;
				bool first_line_local = true;

				while (pos < line.size()) {
					size_t line_start = pos;
					size_t len = 0;
					size_t last_space = std::string_view::npos;

					size_t limit = 128 - prefix.size();

					while (pos < line.size() && len < limit) {
						if (std::isspace(static_cast<unsigned char>(line[pos]))) {
							last_space = pos;
						}
						++pos;
						++len;
					}

					if (pos < line.size() && last_space != std::string_view::npos &&
						last_space > line_start)
					{
						pos = last_space + 1;
						len = last_space - line_start;
					}

					while (pos < line.size() && std::isspace(static_cast<unsigned char>(line[pos])))
					{
						++pos;
					}

					if (first_line_global) {
						ss << prefix;
						first_line_global = false;
					}
					else if (first_line_local) {
						ss << continuation;
					}
					else {
						ss << continuation;
					}

					ss << line.substr(line_start, len) << '\n';
					first_line_local = false;
				}
			}

			start = end + 1;
		}
	}

	return ss.str();
}



void shark::str::forcibly_print_vector(
	const std::vector<std::string>& input)
{
	// std::cout << "\tdebug forcely_print_vector: ";
	for (size_t i = 0; i < input.size(); ++i) {
		// std::cout << i << ":";
		forcibly_print_string(input.at(i));
		std::cout << ", ";
	}
	// std::cout << std::endl;
}
void shark::str::forcibly_print_string(
	const std::string& input)
{
	// std::cout << "";
	for (size_t i = 0; i < input.size(); ++i) {
		std::cout << (char)(*(input.data() + i));
	}
	// std::cout << std::endl;
}


void shark::str::indented_print(
	std::string_view s, char c, int n)
{}

std::string shark::str::indent(
	std::string_view s, char c, int n)
{
	if (s.empty() || n <= 0)
		return std::string(s);
	std::string prefix(n, c);
	std::string result;
	result.reserve(s.size() + prefix.size() * 32); // safe overestimate

	size_t pos = 0;
	while (pos < s.size()) {
		size_t nl = s.find('\n', pos);
		if (nl == std::string_view::npos) {
			result += prefix;
			result += s.substr(pos);
			break;
		}
		result += prefix;
		result += s.substr(pos, nl - pos + 1);
		pos = nl + 1;
	}
	return result;
}

namespace {

std::string toml_trim(std::string s) {
	auto first = s.find_first_not_of(" \t\r\n");
	if (first == std::string::npos)
		return {};
	return s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
}

std::string toml_unquote(const std::string& value, size_t line) {
	if (value.size() < 2 || value.front() != '"' || value.back() != '"')
		throw std::runtime_error("TOML line " + std::to_string(line) + ": expected string");
	std::string out;
	for (size_t i = 1; i + 1 < value.size(); ++i) {
		if (value[i] != '\\') {
			out += value[i];
			continue;
		}
		if (++i + 1 >= value.size())
			throw std::runtime_error("invalid TOML escape");
		switch (value[i]) {
		case 'n': out += '\n'; break;
		case 'r': out += '\r'; break;
		case 't': out += '\t'; break;
		case '"': out += '"'; break;
		case '\\': out += '\\'; break;
		default: throw std::runtime_error("invalid TOML escape");
		}
	}
	return out;
}

shark::toml::Toml parse_toml_scalar(const std::string& value, size_t line) {
	using Toml = shark::toml::Toml;
	if (value.empty())
		throw std::runtime_error("TOML line " + std::to_string(line) + ": empty value");
	if (value.front() == '"')
		return Toml(toml_unquote(value, line));
	if (value == "true")
		return Toml(true);
	if (value == "false")
		return Toml(false);

	int64_t integer_value;
	auto [p, ec] = std::from_chars(value.data(), value.data() + value.size(), integer_value);
	if (ec == std::errc{} && p == value.data() + value.size())
		return Toml(integer_value);

	try {
		size_t n;
		double real_value = std::stod(value, &n);
		if (n == value.size())
			return Toml(real_value);
	} catch (...) {}

	throw std::runtime_error("TOML line " + std::to_string(line) + ": unsupported value");
}

shark::toml::Toml::Table& ensure_toml_table(shark::toml::Toml& node) {
	if (!node.is_table())
		node = shark::toml::Toml(shark::toml::Toml::Table{});
	return node.table();
}

} // namespace

bool shark::toml::Toml::is_table() const { return std::holds_alternative<Table>(value_); }
bool shark::toml::Toml::is_string() const { return std::holds_alternative<std::string>(value_); }
bool shark::toml::Toml::is_integer() const { return std::holds_alternative<int64_t>(value_); }
bool shark::toml::Toml::is_real() const { return std::holds_alternative<double>(value_); }
bool shark::toml::Toml::is_boolean() const { return std::holds_alternative<bool>(value_); }

const shark::toml::Toml& shark::toml::Toml::at(const std::string& key) const {
	const auto& t = table();
	auto it = t.find(key);
	if (it == t.end())
		throw std::runtime_error("missing TOML key: " + key);
	return it->second;
}

shark::toml::Toml& shark::toml::Toml::at(const std::string& key) {
	auto& t = table();
	auto it = t.find(key);
	if (it == t.end())
		throw std::runtime_error("missing TOML key: " + key);
	return it->second;
}

const shark::toml::Toml* shark::toml::Toml::find(const std::string& key) const {
	if (!is_table())
		return nullptr;
	const auto& t = std::get<Table>(value_);
	auto it = t.find(key);
	return it == t.end() ? nullptr : &it->second;
}

shark::toml::Toml* shark::toml::Toml::find(const std::string& key) {
	if (!is_table())
		return nullptr;
	auto& t = std::get<Table>(value_);
	auto it = t.find(key);
	return it == t.end() ? nullptr : &it->second;
}

const shark::toml::Toml::Table& shark::toml::Toml::table() const {
	if (!is_table())
		throw std::runtime_error("expected TOML table");
	return std::get<Table>(value_);
}

shark::toml::Toml::Table& shark::toml::Toml::table() {
	if (!is_table())
		throw std::runtime_error("expected TOML table");
	return std::get<Table>(value_);
}

const std::string& shark::toml::Toml::string() const {
	if (!is_string())
		throw std::runtime_error("expected TOML string");
	return std::get<std::string>(value_);
}

int64_t shark::toml::Toml::integer() const {
	if (!is_integer())
		throw std::runtime_error("expected TOML integer");
	return std::get<int64_t>(value_);
}

double shark::toml::Toml::real() const {
	if (is_real())
		return std::get<double>(value_);
	if (is_integer())
		return static_cast<double>(std::get<int64_t>(value_));
	throw std::runtime_error("expected TOML number");
}

bool shark::toml::Toml::boolean() const {
	if (!is_boolean())
		throw std::runtime_error("expected TOML boolean");
	return std::get<bool>(value_);
}

shark::toml::Toml shark::toml::parse(std::string_view text) {
	Toml root(Toml::Table{});
	Toml* current = &root;
	std::istringstream in{std::string(text)};
	std::string line;
	for (size_t line_no = 1; std::getline(in, line); ++line_no) {
		bool quoted = false;
		for (size_t i = 0; i < line.size(); ++i) {
			if (line[i] == '"' && (i == 0 || line[i - 1] != '\\'))
				quoted = !quoted;
			if (line[i] == '#' && !quoted) {
				line.resize(i);
				break;
			}
		}
		line = toml_trim(line);
		if (line.empty())
			continue;

		if (line.front() == '[' && line.back() == ']') {
			auto section = toml_trim(line.substr(1, line.size() - 2));
			if (section.empty())
				throw std::runtime_error("empty TOML section");
			current = &root;
			size_t begin = 0;
			while (begin <= section.size()) {
				auto end = section.find('.', begin);
				auto key = toml_trim(section.substr(begin, end - begin));
				if (key.empty())
					throw std::runtime_error(
							"TOML line " + std::to_string(line_no) + ": empty section key");
				auto& table = ensure_toml_table(*current);
				auto [it, inserted] = table.try_emplace(key, Toml(Toml::Table{}));
				if (!it->second.is_table())
					throw std::runtime_error(
							"TOML line " + std::to_string(line_no) + ": section conflicts with value");
				current = &it->second;
				if (end == std::string::npos)
					break;
				begin = end + 1;
			}
			continue;
		}

		auto eq = line.find('=');
		if (eq == std::string::npos)
			throw std::runtime_error(
					"TOML line " + std::to_string(line_no) + ": expected key=value");
		auto key = toml_trim(line.substr(0, eq));
		auto value = toml_trim(line.substr(eq + 1));
		if (key.empty())
			throw std::runtime_error("TOML line " + std::to_string(line_no) + ": empty key");

		auto& table = ensure_toml_table(*current);
		if (!table.emplace(key, parse_toml_scalar(value, line_no)).second)
			throw std::runtime_error("duplicate TOML key: " + key);
	}
	return root;
}

shark::toml::Toml shark::toml::parse_toml(const std::filesystem::path& path) {
	std::ifstream in(path);
	if (!in)
		throw std::runtime_error("cannot open config: " + path.string());
	return parse(std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()));
}

namespace {
using Json = shark::json::Json;
	class Parser {
	public:
		explicit Parser(std::string_view input) :
			input_(input) {
		}

		Json parse() {
			auto value = parse_value();
			ws();
			if (pos_ != input_.size())
				fail("trailing data");
			return value;
		}

	private:
		std::string_view input_;
		size_t pos_ = 0;

		[[noreturn]] void fail(const char* what) const {
			throw std::runtime_error("JSON byte " + std::to_string(pos_) + ": " + what);
		}

		void ws() {
			while (pos_ < input_.size() && std::string_view(" \t\r\n").find(input_[pos_]) != std::string_view::npos)
				++pos_;
		}

		bool take(char c) {
			ws();
			if (pos_ < input_.size() && input_[pos_] == c) {
				++pos_;
				return true;
			}
			return false;
		}

		void literal(std::string_view x) {
			if (input_.substr(pos_, x.size()) != x)
				fail("invalid literal");
			pos_ += x.size();
		}

		Json parse_value() {
			ws();
			if (pos_ == input_.size())
				fail("expected value");
			switch (input_[pos_]) {
				case 'n': literal("null");
					return Json(nullptr);
				case 't': literal("true");
					return Json(true);
				case 'f': literal("false");
					return Json(false);
				case '"': return Json(parse_string());
				case '[': return Json(parse_array());
				case '{': return Json(parse_object());
				default: return Json(parse_number());
			}
		}

		static void utf8(std::string& out, uint32_t cp) {
			if (cp <= 0x7f)
				out += static_cast<char>(cp);
			else if (cp <= 0x7ff) {
				out += static_cast<char>(0xc0 | cp >> 6);
				out += static_cast<char>(0x80 | (cp & 63));
			} else if (cp <= 0xffff) {
				out += static_cast<char>(0xe0 | cp >> 12);
				out += static_cast<char>(0x80 | ((cp >> 6) & 63));
				out += static_cast<char>(0x80 | (cp & 63));
			} else {
				out += static_cast<char>(0xf0 | cp >> 18);
				out += static_cast<char>(0x80 | ((cp >> 12) & 63));
				out += static_cast<char>(0x80 | ((cp >> 6) & 63));
				out += static_cast<char>(0x80 | (cp & 63));
			}
		}

		uint32_t hex4() {
			uint32_t x = 0;
			for (int n = 0; n < 4; ++n) {
				if (pos_ >= input_.size())
					fail("short unicode escape");
				char c = input_[pos_++];
				x <<= 4;
				if (c >= '0' && c <= '9')
					x += c - '0';
				else if (c >= 'a' && c <= 'f')
					x += c - 'a' + 10;
				else if (c >= 'A' && c <= 'F')
					x += c - 'A' + 10;
				else
					fail("bad unicode escape");
			}
			return x;
		}

		std::string parse_string() {
			if (!take('"'))
				fail("expected string");
			std::string out;
			while (pos_ < input_.size()) {
				char c = input_[pos_++];
				if (c == '"')
					return out;
				if (static_cast<unsigned char>(c) < 0x20)
					fail("control in string");
				if (c != '\\') {
					out += c;
					continue;
				}
				if (pos_ == input_.size())
					fail("short escape");
				c = input_[pos_++];
				switch (c) {
					case '"': out += '"';
						break;
					case '\\': out += '\\';
						break;
					case '/': out += '/';
						break;
					case 'b': out += '\b';
						break;
					case 'f': out += '\f';
						break;
					case 'n': out += '\n';
						break;
					case 'r': out += '\r';
						break;
					case 't': out += '\t';
						break;
					case 'u': {
						auto cp = hex4();
						if (cp >= 0xd800 && cp <= 0xdbff) {
							if (input_.substr(pos_, 2) != "\\u")
								fail("missing low surrogate");
							pos_ += 2;
							auto lo = hex4();
							if (lo < 0xdc00 || lo > 0xdfff)
								fail("bad low surrogate");
							cp = 0x10000 + ((cp - 0xd800) << 10) + (lo - 0xdc00);
						}
						utf8(out, cp);
						break;
					}
					default: fail("bad escape");
				}
			}
			fail("unterminated string");
		}

		double parse_number() {
			ws();
			auto first = input_.data() + pos_;
			char* end = nullptr;
			auto x = std::strtod(first, &end);
			if (end == first)
				fail("expected number");
			pos_ += static_cast<size_t>(end - first);
			if (!std::isfinite(x))
				fail("non-finite number");
			return x;
		}

		Json::Array parse_array() {
			take('[');
			Json::Array a;
			if (take(']'))
				return a;
			do {
				a.push_back(parse_value());
			} while (take(','));
			if (!take(']'))
				fail("expected ]");
			return a;
		}

		Json::Object parse_object() {
			take('{');
			Json::Object o;
			if (take('}'))
				return o;
			do {
				ws();
				auto k = parse_string();
				if (!take(':'))
					fail("expected :");
				if (!o.emplace(std::move(k), parse_value()).second)
					fail("duplicate key");
			} while (take(','));
			if (!take('}'))
				fail("expected }");
			return o;
		}
	};
} // namespace

shark::json::Json shark::json::Json::parse(std::string_view text) { return Parser(text).parse(); }

bool shark::json::Json::is_null() const { return std::holds_alternative<std::nullptr_t>(value_); }

bool shark::json::Json::is_string() const { return std::holds_alternative<std::string>(value_); }

const std::string& shark::json::Json::string() const { return std::get<std::string>(value_); }

double shark::json::Json::number() const { return std::get<double>(value_); }

const shark::json::Json::Array& shark::json::Json::array() const { return std::get<Array>(value_); }

const shark::json::Json::Object& shark::json::Json::object() const { return std::get<Object>(value_); }

const shark::json::Json& shark::json::Json::at(const std::string& key) const {
	auto i = object().find(key);
	if (i == object().end())
		throw std::runtime_error("missing JSON key: " + key);
	return i->second;
}

const shark::json::Json* shark::json::Json::find(const std::string& key) const {
	auto* o = std::get_if<Object>(&value_);
	if (!o)
		return nullptr;
	auto i = o->find(key);
	return i == o->end() ? nullptr : &i->second;
}

