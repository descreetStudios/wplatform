#include <wplatform/version.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

namespace
{
	constexpr bool matches_macros(const wplatform::Version& v)
	{
		return v.major == WPLATFORM_VERSION_MAJOR
			&& v.minor == WPLATFORM_VERSION_MINOR
			&& v.patch == WPLATFORM_VERSION_PATCH;
	}

	static_assert(matches_macros(wplatform::CurrentVersion));
	static_assert(wplatform::CurrentVersion == wplatform::Version{
		WPLATFORM_VERSION_MAJOR, WPLATFORM_VERSION_MINOR, WPLATFORM_VERSION_PATCH});
	static_assert(wplatform::CurrentVersionString == WPLATFORM_VERSION_STRING);
} // namespace

TEST_CASE("version is available through macros", "[version]")
{
	REQUIRE(WPLATFORM_VERSION_MAJOR == WPLATFORM_EXPECTED_VERSION_MAJOR);
	REQUIRE(WPLATFORM_VERSION_MINOR == WPLATFORM_EXPECTED_VERSION_MINOR);
	REQUIRE(WPLATFORM_VERSION_PATCH == WPLATFORM_EXPECTED_VERSION_PATCH);
	REQUIRE(std::string{ WPLATFORM_VERSION_STRING } == WPLATFORM_EXPECTED_VERSION);
}

TEST_CASE("version is available through constants", "[version]")
{
	REQUIRE(wplatform::CurrentVersion.major == WPLATFORM_VERSION_MAJOR);
	REQUIRE(wplatform::CurrentVersion.minor == WPLATFORM_VERSION_MINOR);
	REQUIRE(wplatform::CurrentVersion.patch == WPLATFORM_VERSION_PATCH);
	REQUIRE(std::string{ wplatform::CurrentVersionString } == WPLATFORM_EXPECTED_VERSION);
}

TEST_CASE("version is available through runtime functions", "[version]")
{
	REQUIRE(wplatform::platform_version() == wplatform::CurrentVersion);
	REQUIRE(std::string{ wplatform::version_string() } == WPLATFORM_EXPECTED_VERSION);
}

TEST_CASE("version is available through formatting", "[version]")
{
	REQUIRE(wplatform::to_string(wplatform::CurrentVersion) == WPLATFORM_EXPECTED_VERSION);

	std::ostringstream out;
	out << wplatform::CurrentVersion;
	REQUIRE(out.str() == WPLATFORM_EXPECTED_VERSION);
}

TEST_CASE("versions compare semantically", "[version]")
{
	REQUIRE(wplatform::Version{ 1, 0, 0 } > wplatform::Version{ 0, 9, 9 });
	REQUIRE(wplatform::Version{ 1, 2, 3 } == wplatform::Version{ 1, 2, 3 });
	REQUIRE(wplatform::CurrentVersion >= wplatform::Version{ 0, 0, 0 });
}
