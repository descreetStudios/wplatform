#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <catch2/catch_test_macros.hpp>

#include <wcore/log.hpp>

#include <array>
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

TEST_CASE("window config applies advanced hints", "[window]")
{
	PlatformGuard guard;

	auto config{ make_config() };
	config.maximized = true;
	config.floating = true;
	config.autoIconify = false;
	config.mousePassthrough = true;
	config.transparentFramebuffer = true;
	config.sizeLimits.minWidth = 200;
	config.sizeLimits.minHeight = 100;

	auto window{ wplatform::window::Window::create(config) };
	REQUIRE(window.has_value());

	REQUIRE(window->maximized());
	REQUIRE(window->floating());
	REQUIRE_FALSE(window->auto_iconify());
	REQUIRE(window->mouse_passthrough());
	REQUIRE(window->transparent_framebuffer());
}

TEST_CASE("window size constraints can be set and cleared", "[window]")
{
	PlatformGuard guard;

	auto window{ wplatform::window::Window::create(make_config()) };
	REQUIRE(window.has_value());

	wplatform::window::SizeLimits limits;
	limits.minWidth = 100;
	limits.maxWidth = 800;
	limits.maxHeight = 600;

	window->set_size_limits(limits);
	window->clear_size_limits();

	window->set_aspect_ratio(16, 9);
	window->clear_aspect_ratio();
}

TEST_CASE("window attributes can be toggled", "[window]")
{
	PlatformGuard guard;

	auto window{ wplatform::window::Window::create(make_config()) };
	REQUIRE(window.has_value());

	REQUIRE_FALSE(window->floating());
	window->set_floating(true);
	REQUIRE(window->floating());
	window->set_floating(false);
	REQUIRE_FALSE(window->floating());

	REQUIRE(window->auto_iconify());
	window->set_auto_iconify(false);
	REQUIRE_FALSE(window->auto_iconify());

	REQUIRE_FALSE(window->mouse_passthrough());
	window->set_mouse_passthrough(true);
	REQUIRE(window->mouse_passthrough());

	// Hover depends on the pointer position, so just exercise the query.
	window->hovered();
}

TEST_CASE("window icons and opacity can be changed", "[window]")
{
	PlatformGuard guard;

	auto window{ wplatform::window::Window::create(make_config()) };
	REQUIRE(window.has_value());

	wplatform::window::Icon icon;
	icon.size = { 2, 2 };
	icon.pixels.assign(2 * 2 * 4, 0);
	REQUIRE(icon.valid());

	REQUIRE(window->set_icon(icon));

	const std::array<wplatform::window::Icon, 2> icons{ icon, icon };
	REQUIRE(window->set_icons(icons));
	REQUIRE(window->set_icons({}));

	REQUIRE(window->opacity() == 1.0f);
	window->set_opacity(0.5f);
	REQUIRE(window->opacity() == 0.5f);
	window->set_opacity(1.0f);

	window->request_attention();
}

TEST_CASE("windows can switch between windowed and fullscreen", "[window]")
{
	PlatformGuard guard;

	auto window{ wplatform::window::Window::create(make_config()) };
	REQUIRE(window.has_value());
	REQUIRE_FALSE(window->fullscreen_monitor().has_value());

	const auto primary{ wplatform::monitor::primary() };
	REQUIRE(primary.has_value());

	const auto mode{ primary->current_mode() };
	REQUIRE(mode.has_value());

	REQUIRE(window->set_fullscreen(*primary, *mode));
	const auto active{ window->fullscreen_monitor() };
	REQUIRE(active.has_value());
	REQUIRE(*active == *primary);
	REQUIRE(window->size().width == mode->size.width);
	REQUIRE(window->size().height == mode->size.height);

	REQUIRE(window->set_windowed({ 20, 30 }, { 400, 300 }));
	REQUIRE_FALSE(window->fullscreen_monitor().has_value());
	REQUIRE(window->size().width == 400);
	REQUIRE(window->size().height == 300);
}

TEST_CASE("invalid window settings are rejected", "[window]")
{
	wcore::log::LogSilencer silence;
	PlatformGuard guard;

	auto window{ wplatform::window::Window::create(make_config()) };
	REQUIRE(window.has_value());

	window->set_aspect_ratio(0, 9);
	window->set_opacity(-0.1f);
	REQUIRE_FALSE(window->set_icon(wplatform::window::Icon{}));

	const auto primary{ wplatform::monitor::primary() };
	REQUIRE(primary.has_value());
	REQUIRE_FALSE(window->set_fullscreen(*primary, wplatform::monitor::VideoMode{}));
	REQUIRE_FALSE(window->set_fullscreen(
		wplatform::monitor::Monitor{}, wplatform::monitor::VideoMode{}));
	REQUIRE_FALSE(window->set_windowed({ 0, 0 }, { 0, 0 }));

	wplatform::window::Window destination{ std::move(*window) };
	REQUIRE(destination.valid());

	window->clear_size_limits();
	window->clear_aspect_ratio();
	window->request_attention();
	window->set_floating(true);
	REQUIRE(window->opacity() == 1.0f);
	REQUIRE_FALSE(window->fullscreen_monitor().has_value());
	REQUIRE_FALSE(window->set_icon(wplatform::window::Icon{}));
}
