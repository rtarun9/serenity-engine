cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

project(serenity-engine LANGUAGES CXX)

# Specify output paths for all configurations.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/debug)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/release)

# Enable hot reload for visual studio (having the code here ensures all targets have this setting applied to them)
if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
	set(CMAKE_CXX_FLAGS_DEBUG "/MDd /ZI /Ob0 /Od /RTC1")
endif()

add_subdirectory(external)
add_subdirectory(serenity-engine)
add_subdirectory(game)