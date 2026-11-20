include(FetchContent)

# Dependencies
FetchContent_Declare(Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG v3.16.1
	GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

if(NOT COMMAND catch_discover_tests)
	if(DEFINED catch2_SOURCE_DIR AND EXISTS "${catch2_SOURCE_DIR}/extras/Catch.cmake")
		list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
	endif()

	include(Catch)
endif()

# Target
add_executable(wplatform_tests
	"smoke_test.cpp"
	"version_test.cpp"
	"dependencies_test.cpp"
	"platform_test.cpp"
	"events_test.cpp"
	"input_test.cpp"
	"monitor_test.cpp"
	"window_test.cpp"
)

target_compile_definitions(wplatform_tests PRIVATE
	WPLATFORM_EXPECTED_VERSION="${PROJECT_VERSION}"
	WPLATFORM_EXPECTED_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
	WPLATFORM_EXPECTED_VERSION_MINOR=${PROJECT_VERSION_MINOR}
	WPLATFORM_EXPECTED_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

target_link_libraries(wplatform_tests PRIVATE
	wplatform::wplatform
	Catch2::Catch2WithMain
)

catch_discover_tests(wplatform_tests DISCOVERY_MODE PRE_TEST)
