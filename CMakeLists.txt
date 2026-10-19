cmake_minimum_required(VERSION 3.31)

project(shark CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED COMPONENTS json)
find_package(OpenGL REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_library(OPUSENC_LIBRARY NAMES opusenc REQUIRED)
find_path(OPUS_INCLUDE_DIR NAMES opus.h PATH_SUFFIXES opus REQUIRED)
find_library(OPUSFILE_LIBRARY NAMES opusfile REQUIRED)
find_path(MPG123_INCLUDE_DIR NAMES mpg123.h REQUIRED)
find_library(MPG123_LIBRARY NAMES mpg123 REQUIRED)
find_path(LAME_INCLUDE_DIR NAMES lame/lame.h lame.h REQUIRED)
find_library(LAME_LIBRARY NAMES mp3lame lame REQUIRED)

add_library(${PROJECT_NAME} STATIC
	src/audio.cpp
	src/external.cpp
	src/image.cpp
	src/legacy.cpp
        src/str.cpp
	src/termux.cpp
)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${OPUS_INCLUDE_DIR}
		${MPG123_INCLUDE_DIR}
		${LAME_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	JPEG::JPEG PNG::PNG
	OpenGL::GL
	${OPUSFILE_LIBRARY}
	${OPUSENC_LIBRARY}
	${MPG123_LIBRARY}
	${LAME_LIBRARY}
)

target_compile_options(${PROJECT_NAME} PRIVATE -O2)
