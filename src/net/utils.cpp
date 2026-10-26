/*
 * Project: shark
 * Repository: https://github.com/leaked32/shark
 *
 * File: shark/src/external.cpp
 *
 * License: MIT
 */

#include <fstream>

#include "shark/str.hpp"
#include "shark/net/utils.hpp"


boost::json::object shark::file::read_json(
	std::string_view path)
{
	namespace json = boost::json;
	auto raw = json::parse(read(path));
	return raw.as_object();
}

void shark::file::dump_json(
	std::string_view path, const boost::json::object& js)
{
	namespace json = boost::json;
	std::string pretty = json::serialize(js);

	std::ofstream(std::string(path)) << pretty;
}
