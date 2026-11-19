#include <wplatform/platform.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <thread>

namespace
{
	struct PlatformGuard
	{
		~PlatformGuard()
		{
			wplatform::platform::shutdown();
		}
	};
} // namespace

TEST_CASE("platform starts uninitialized", "[platform]")
{
	REQUIRE_FALSE(wplatform::platform::initialized());
	REQUIRE_FALSE(wplatform::platform::is_main_thread());
	REQUIRE(wplatform::platform::backend() == wplatform::platform::Backend::any);
	REQUIRE(std::string{ wplatform::platform::backend_name() } == "any");
}

TEST_CASE("platform initializes with the null backend", "[platform]")
{
	PlatformGuard guard;

	wplatform::platform::Config config;
	config.backend = wplatform::platform::Backend::null;

	REQUIRE(wplatform::platform::backend_supported(wplatform::platform::Backend::null));
	REQUIRE(wplatform::platform::init(config));
	REQUIRE(wplatform::platform::initialized());
	REQUIRE(wplatform::platform::backend() == wplatform::platform::Backend::null);
	REQUIRE(std::string{ wplatform::platform::backend_name() } == "null");
	REQUIRE(wplatform::platform::is_main_thread());
	REQUIRE(wplatform::platform::glfw_version().major >= 3);
	REQUIRE(wplatform::platform::glfw_versions_match());
}

TEST_CASE("platform init and shutdown are idempotent", "[platform]")
{
	PlatformGuard guard;

	wplatform::platform::Config config;
	config.backend = wplatform::platform::Backend::null;

	REQUIRE(wplatform::platform::init(config));
	REQUIRE(wplatform::platform::init(config));
	REQUIRE(wplatform::platform::initialized());

	wplatform::platform::shutdown();
	REQUIRE_FALSE(wplatform::platform::initialized());
	REQUIRE_FALSE(wplatform::platform::is_main_thread());

	wplatform::platform::shutdown();
}

TEST_CASE("platform rejects non-main-thread init", "[platform]")
{
	PlatformGuard guard;

	wplatform::platform::Config config;
	config.backend = wplatform::platform::Backend::null;
	REQUIRE(wplatform::platform::init(config));

	bool initialized{ true };
	std::thread worker([&initialized]()
	{
		initialized = wplatform::platform::init();
	});
	worker.join();

	REQUIRE_FALSE(initialized);
	REQUIRE(wplatform::platform::initialized());
}

TEST_CASE("unsupported backends fail cleanly", "[platform]")
{
	if (wplatform::platform::backend_supported(wplatform::platform::Backend::wayland))
	{
		SUCCEED("Wayland is supported here; no unavailable backend to test");
		return;
	}

	wplatform::platform::Config config;
	config.backend = wplatform::platform::Backend::wayland;

	REQUIRE_FALSE(wplatform::platform::init(config));
	REQUIRE_FALSE(wplatform::platform::initialized());
}
