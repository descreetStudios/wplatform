#include <wplatform/events.hpp>
#include <wplatform/monitor.hpp>
#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <catch2/catch_test_macros.hpp>

#include <wcore/log.hpp>

#include <algorithm>
#include <string>
#include <vector>

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

	bool contains(const std::vector<wplatform::monitor::VideoMode>& modes,
		const wplatform::monitor::VideoMode& mode)
	{
		return std::find(modes.begin(), modes.end(), mode) != modes.end();
	}
} // namespace

TEST_CASE("monitor queries require an initialized platform", "[monitor]")
{
	wcore::log::LogSilencer silence;
	wplatform::platform::shutdown();

	REQUIRE(wplatform::monitor::all().empty());
	REQUIRE(wplatform::monitor::count() == 0);
	REQUIRE_FALSE(wplatform::monitor::primary().has_value());
	REQUIRE_FALSE(wplatform::monitor::at({ 0, 0 }).has_value());

	const wplatform::monitor::Monitor monitor;
	REQUIRE_FALSE(monitor.valid());
	REQUIRE_FALSE(static_cast<bool>(monitor));
	REQUIRE(monitor.native_handle() == nullptr);
	REQUIRE(monitor.name().empty());
	REQUIRE_FALSE(monitor.current_mode().has_value());
	REQUIRE(monitor.modes().empty());
	REQUIRE(monitor.work_area().size.width == 0);
	REQUIRE(monitor.work_area().size.height == 0);
	REQUIRE(monitor.content_scale().x == 1.0f);
	REQUIRE(monitor.content_scale().y == 1.0f);
	REQUIRE(monitor.gamma_ramp().size() == 0);
}

TEST_CASE("the null backend exposes a primary monitor", "[monitor]")
{
	PlatformGuard guard;

	REQUIRE(wplatform::monitor::count() >= 1);

	const auto monitors{ wplatform::monitor::all() };
	REQUIRE_FALSE(monitors.empty());

	const auto primary{ wplatform::monitor::primary() };
	REQUIRE(primary.has_value());
	REQUIRE(primary->valid());
	REQUIRE(primary->native_handle() != nullptr);
	REQUIRE_FALSE(primary->name().empty());

	const auto mode{ primary->current_mode() };
	REQUIRE(mode.has_value());
	REQUIRE(mode->size.width > 0);
	REQUIRE(mode->size.height > 0);
	REQUIRE(mode->redBits == 8);
	REQUIRE(mode->greenBits == 8);
	REQUIRE(mode->blueBits == 8);
	REQUIRE(mode->refreshRate > 0);

	const auto area{ primary->work_area() };
	REQUIRE(area.size.width > 0);
	REQUIRE(area.size.height > 0);

	const auto scale{ primary->content_scale() };
	REQUIRE(scale.x > 0.0f);
	REQUIRE(scale.y > 0.0f);

	const auto physical{ primary->physical_size() };
	REQUIRE(physical.width > 0);
	REQUIRE(physical.height > 0);
}

TEST_CASE("monitors can be found by position", "[monitor]")
{
	PlatformGuard guard;

	const auto primary{ wplatform::monitor::primary() };
	REQUIRE(primary.has_value());

	const auto mode{ primary->current_mode() };
	REQUIRE(mode.has_value());

	const auto origin{ primary->position() };
	const auto found{ wplatform::monitor::at(origin) };
	REQUIRE(found.has_value());
	REQUIRE(*found == *primary);

	const wplatform::Position outside{
		origin.x + static_cast<i32>(mode->size.width),
		origin.y + static_cast<i32>(mode->size.height) };
	REQUIRE_FALSE(wplatform::monitor::at(outside).has_value());
}

TEST_CASE("monitor modes include the current mode", "[monitor]")
{
	PlatformGuard guard;

	const auto primary{ wplatform::monitor::primary() };
	REQUIRE(primary.has_value());

	const auto current{ primary->current_mode() };
	REQUIRE(current.has_value());

	const auto modes{ primary->modes() };
	REQUIRE_FALSE(modes.empty());
	REQUIRE(contains(modes, *current));
}

TEST_CASE("monitor gamma ramps can be read and written", "[monitor]")
{
	PlatformGuard guard;

	auto primary{ wplatform::monitor::primary() };
	REQUIRE(primary.has_value());

	const auto ramp{ primary->gamma_ramp() };
	REQUIRE(ramp.valid());
	REQUIRE(ramp.size() > 0);
	REQUIRE(ramp.green.size() == ramp.size());
	REQUIRE(ramp.blue.size() == ramp.size());

	REQUIRE(primary->set_gamma_ramp(ramp));

	primary->set_gamma(1.0f);
	const auto adjusted{ primary->gamma_ramp() };
	REQUIRE(adjusted.valid());
	REQUIRE(adjusted.size() == ramp.size());

	REQUIRE_FALSE(primary->set_gamma_ramp(wplatform::monitor::GammaRamp{}));
}

TEST_CASE("monitor events can be delivered to a window dispatcher", "[monitor][events]")
{
	PlatformGuard guard;

	const auto primary{ wplatform::monitor::primary() };
	REQUIRE(primary.has_value());

	wplatform::window::Config config;
	config.title = "wplatform monitor test";
	config.size = { 320, 240 };

	auto window{ wplatform::window::Window::create(config) };
	REQUIRE(window.has_value());

	bool connected{ false };
	bool disconnected{ false };
	std::string disconnectedName;

	window->dispatcher()->add_listener(wplatform::events::Type::monitor_connected,
		[&](const wplatform::events::Event& event)
		{
			const auto* data{ event.get<wplatform::events::MonitorConnected>() };
			if (data != nullptr)
				connected = data->handle == *primary;
		});

	window->dispatcher()->add_listener(wplatform::events::Type::monitor_disconnected,
		[&](const wplatform::events::Event& event)
		{
			const auto* data{ event.get<wplatform::events::MonitorDisconnected>() };
			if (data != nullptr)
			{
				disconnected = true;
				disconnectedName = data->name;
			}
		});

	window->dispatcher()->dispatch(wplatform::events::Event{
		wplatform::events::Type::monitor_connected,
		wplatform::events::MonitorConnected{ *primary } });

	window->dispatcher()->dispatch(wplatform::events::Event{
		wplatform::events::Type::monitor_disconnected,
		wplatform::events::MonitorDisconnected{ *primary, "Test Monitor" } });

	REQUIRE(connected);
	REQUIRE(disconnected);
	REQUIRE(disconnectedName == "Test Monitor");
}
