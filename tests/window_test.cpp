#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <catch2/catch_test_macros.hpp>

#include <wcore/log.hpp>

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
		config.title = "wplatform test";
		config.size = { 640, 480 };
		return config;
	}
} // namespace

TEST_CASE("window creation requires an initialized platform", "[window]")
{
	wcore::log::LogSilencer silence;
	wplatform::platform::shutdown();

	REQUIRE_FALSE(wplatform::platform::initialized());
	REQUIRE_FALSE(wplatform::window::Window::create().has_value());
}

TEST_CASE("window creates with the requested properties", "[window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());
	REQUIRE(window->valid());
	REQUIRE(window->native_handle() != nullptr);

	const auto size = window->size();
	REQUIRE(size.width == 640);
	REQUIRE(size.height == 480);

	const auto framebuffer = window->framebuffer_size();
	REQUIRE(framebuffer.width == 640);
	REQUIRE(framebuffer.height == 480);

	REQUIRE(std::string{ window->title() } == "wplatform test");
	REQUIRE(window->visible());
	REQUIRE(window->focused());
	REQUIRE(window->resizable());

	const auto scale = window->content_scale();
	REQUIRE(scale.x > 0.0f);
	REQUIRE(scale.y > 0.0f);
}

TEST_CASE("window properties can be changed", "[window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());

	window->set_size(800, 600);
	REQUIRE(window->size().width == 800);
	REQUIRE(window->size().height == 600);

	window->set_position(100, 200);
	REQUIRE(window->position().x == 100);
	REQUIRE(window->position().y == 200);

	window->set_title("renamed");
	REQUIRE(std::string{ window->title() } == "renamed");

	window->set_resizable(false);
	REQUIRE_FALSE(window->resizable());

	window->hide();
	REQUIRE_FALSE(window->visible());

	window->show();
	REQUIRE(window->visible());
}

TEST_CASE("window state changes are observable", "[window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());

	window->iconify();
	REQUIRE(window->iconified());

	window->restore();
	REQUIRE_FALSE(window->iconified());

	window->maximize();
	REQUIRE(window->maximized());

	window->restore();
	REQUIRE_FALSE(window->maximized());
}

TEST_CASE("window close requests are observable", "[window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());

	REQUIRE_FALSE(window->should_close());

	window->request_close();
	REQUIRE(window->should_close());
}

TEST_CASE("windows are movable", "[window]")
{
	PlatformGuard guard;

	auto first = wplatform::window::Window::create(make_config());
	auto second = wplatform::window::Window::create(make_config());
	REQUIRE(first.has_value());
	REQUIRE(second.has_value());

	auto* handle = first->native_handle();

	wplatform::window::Window moved{ std::move(*first) };
	REQUIRE(moved.valid());
	REQUIRE(moved.native_handle() == handle);
	REQUIRE_FALSE(first->valid());

	*second = std::move(moved);
	REQUIRE(second->valid());
	REQUIRE(second->native_handle() == handle);
	REQUIRE_FALSE(moved.valid());
}
