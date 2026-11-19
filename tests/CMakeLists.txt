include(FetchContent)

# Dependencies
FetchContent_Declare(Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG v3.16.1
	GIT_SHALLOW TRUE
	FIND_PACKAGE_ARGS 3 CONFIG
)
FetchContent_MakeAvailable(Catch2)
include(Catch)

# Target
add_executable(wplatform_tests
	"smoke_test.cpp"
	"version_test.cpp"
	"dependencies_test.cpp"
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
