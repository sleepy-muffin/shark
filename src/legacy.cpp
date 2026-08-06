/*
 * Project: shark-arts
 * Repository: https://github.com/leaked32/shark-arts
 *
 * File: shark/src/legacy.cpp
 *
 * License: MIT
 */

#include "shark/legacy.hpp"

#ifdef __linux__

std::string shark::find_system_font(
	std::string_view name)
{
	std::string command = "fc-match -f '%{file}' \"" + std::string(name) + "\"";
	
	std::array<char, 256> buffer = {};
	
	std::string result;
	
	FILE* pipe = popen(command.c_str(), "r");
	
	if (!pipe) {
		return {};
	}
	
	while (fgets(buffer.data(), buffer.size(), pipe)) {
		result += buffer.data();
	}
	
	pclose(pipe);
	
	return result;
}

#endif
