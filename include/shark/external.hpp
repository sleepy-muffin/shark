/*
 * Project: shark-arts
 * Repository: https://github.com/leaked32/shark-arts
 *
 * File: shark/include/shark/external.hpp
 *
 * License: MIT
 */

#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include "shark/common.hpp"
#include <boost/json.hpp>

SHARK_BEGIN
namespace file
{

boost::json::object read_json(std::string_view path);
void dump_json(std::string_view path, const boost::json::object& js);
} // namespace file

SHARK_END
