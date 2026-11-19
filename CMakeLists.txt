cmake_minimum_required(VERSION 3.24)
project(wplatform VERSION 0.1.0 LANGUAGES CXX)

# Options
option(WPLATFORM_BUILD_TESTS "Build wplatform tests" ${PROJECT_IS_TOP_LEVEL})
option(WPLATFORM_USE_SYSTEM_GLFW "Use an installed GLFW instead of fetching it" OFF)
option(WPLATFORM_USE_SYSTEM_EVENTPP "Use an installed eventpp instead of fetching it" OFF)

if(PROJECT_IS_TOP_LEVEL)
	set(CMAKE_CXX_EXTENSIONS OFF)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Dependencies
include(cmake/dependencies.cmake)

# Library
add_library(wplatform STATIC)
add_library(wplatform::wplatform ALIAS wplatform)

target_sources(wplatform PRIVATE
	"src/version.cpp"
	"src/platform.cpp"
)

# Version
configure_file(
	cmake/version.hpp.in
	${CMAKE_CURRENT_BINARY_DIR}/include/wplatform/version.generated.hpp
	@ONLY)

target_include_directories(wplatform PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include
)

target_link_libraries(wplatform PUBLIC
	wcore::wcore
	glfw
	eventpp::eventpp
)

target_compile_features(wplatform PUBLIC cxx_std_20)

target_compile_options(wplatform PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
	$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

set_target_properties(wplatform
	PROPERTIES POSITION_INDEPENDENT_CODE ON) # For Linux/macOS shared modules

# Tests
if(WPLATFORM_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
