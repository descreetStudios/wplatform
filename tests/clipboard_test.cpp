#include <wplatform/clipboard.hpp>
#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <catch2/catch_test_macros.hpp>

#include <wcore/log.hpp>

#include <optional>
#include <string>
#include <utility>

namespace
{
	struct PlatformGuard
	{
		PlatformGuard()
		{
			wplatform::platform::Config config;
			config.backend = wplatform::platform::Backend::null;
			REQUIRE(wplatform::platform::init(config));
		}

		~PlatformGuard()
		{
			wplatform::platform::shutdown();
		}
	};

	wplatform::window::Config make_config()
	{
		wplatform::window::Config config;
		config.title = "wplatform clipboard test";
		config.size = { 320, 240 };
		return config;
	}
} // namespace

TEST_CASE("clipboard operations require an initialized platform", "[clipboard]")
{
	wcore::log::LogSilencer silence;
	std::optional<wplatform::window::Window> window;

	{
		PlatformGuard guard;
		window = wplatform::window::Window::create(make_config());
		REQUIRE(window.has_value());

		// Leave an empty Window shell behind so it can outlive the platform.
		wplatform::window::Window destination{ std::move(*window) };
		REQUIRE(destination.valid());
		REQUIRE(window->native_handle() == nullptr);
	}

	REQUIRE_FALSE(wplatform::platform::initialized());

	REQUIRE_FALSE(wplatform::clipboard::get(*window).has_value());
	REQUIRE_FALSE(wplatform::clipboard::set(*window, "text"));
}

TEST_CASE("clipboard operations require a valid window", "[clipboard]")
{
	wcore::log::LogSilencer silence;
	PlatformGuard guard;

	auto window{ wplatform::window::Window::create(make_config()) };
	REQUIRE(window.has_value());

	wplatform::window::Window destination{ std::move(*window) };
	REQUIRE(destination.valid());
	REQUIRE(window->native_handle() == nullptr);

	REQUIRE_FALSE(wplatform::clipboard::get(*window).has_value());
	REQUIRE_FALSE(wplatform::clipboard::set(*window, "text"));
}

TEST_CASE("clipboard text can be written and read back", "[clipboard]")
{
	PlatformGuard guard;

	auto window{ wplatform::window::Window::create(make_config()) };
	REQUIRE(window.has_value());

	const std::string text{ "wplatform clipboard \xC3\xA9\nsecond line" };
	REQUIRE(wplatform::clipboard::set(*window, text));

	const auto result{ wplatform::clipboard::get(*window) };
	REQUIRE(result.has_value());
	REQUIRE(*result == text);

	REQUIRE(wplatform::clipboard::set(*window, "replacement"));
	const auto replaced{ wplatform::clipboard::get(*window) };
	REQUIRE(replaced.has_value());
	REQUIRE(*replaced == "replacement");
}

TEST_CASE("empty clipboard text is distinct from an unavailable clipboard", "[clipboard]")
{
	PlatformGuard guard;

	auto window{ wplatform::window::Window::create(make_config()) };
	REQUIRE(window.has_value());

	REQUIRE_FALSE(wplatform::clipboard::get(*window).has_value());

	REQUIRE(wplatform::clipboard::set(*window, ""));
	const auto result{ wplatform::clipboard::get(*window) };
	REQUIRE(result.has_value());
	REQUIRE(result->empty());
}
