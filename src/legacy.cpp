/*
 * Project: shark-arts
 * Repository: https://github.com/leaked32/shark-arts
 *
 * File: shark/src/legacy.cpp
 *
 * License: MIT
 */

#include "shark/legacy.hpp"

void shark::forcibly_print_vector(
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

void shark::indented_print(
	std::string_view s, char c, int n)
{}

std::string shark::indent(
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

void shark::forcibly_print_string(
	const std::string& input)
{
	// std::cout << "";
	for (size_t i = 0; i < input.size(); ++i) {
		std::cout << (char)(*(input.data() + i));
	}
	// std::cout << std::endl;
}
