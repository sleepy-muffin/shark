/*
 * Project: shark
 * Repository: https://github.com/leaked32/shark
 *
 * File: shark/include/shark/audio.hpp
 *
 * License: MIT
 */

#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif

#include <filesystem>
#include <optional>

#include <GL/gl.h>

namespace mmc::media
{
	
	struct texture
	{
		GLuint id = 0;
		int width = 0;
		int height = 0;
	};
	
	std::optional<texture> load_texture(const std::filesystem::path& path);
	std::optional<texture> load_png_texture(const std::filesystem::path& path);
	std::optional<texture> load_jpeg_texture(const std::filesystem::path& path);
	
}
