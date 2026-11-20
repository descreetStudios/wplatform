#include <wplatform/events.hpp>
#include <wplatform/joystick.hpp>
#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <catch2/catch_test_macros.hpp>

#include <wcore/log.hpp>

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
} // namespace

TEST_CASE("joystick queries require an initialized platform", "[joystick]")
{
	wcore::log::LogSilencer silence;
	wplatform::platform::shutdown();

	REQUIRE_FALSE(wplatform::joystick::present(wplatform::joystick::Id::joy_1));
	REQUIRE_FALSE(wplatform::joystick::name(wplatform::joystick::Id::joy_1).has_value());
	REQUIRE_FALSE(wplatform::joystick::guid(wplatform::joystick::Id::joy_1).has_value());
	REQUIRE_FALSE(wplatform::joystick::state(wplatform::joystick::Id::joy_1).has_value());
	REQUIRE_FALSE(wplatform::joystick::is_gamepad(wplatform::joystick::Id::joy_1));
	REQUIRE_FALSE(wplatform::joystick::gamepad_name(wplatform::joystick::Id::joy_1).has_value());
	REQUIRE_FALSE(wplatform::joystick::gamepad_state(wplatform::joystick::Id::joy_1).has_value());

	REQUIRE(wplatform::joystick::connected().empty());
	REQUIRE(wplatform::joystick::connected_count() == 0);
	REQUIRE_FALSE(wplatform::joystick::update_mappings(""));
}

TEST_CASE("the null backend reports no joysticks", "[joystick]")
{
	PlatformGuard guard;

	REQUIRE(wplatform::joystick::connected().empty());
	REQUIRE(wplatform::joystick::connected_count() == 0);

	for (i32 i{ 0 }; i < static_cast<i32>(wplatform::joystick::Count); ++i)
	{
		const auto id{ static_cast<wplatform::joystick::Id>(i) };

		REQUIRE_FALSE(wplatform::joystick::present(id));
		REQUIRE_FALSE(wplatform::joystick::name(id).has_value());
		REQUIRE_FALSE(wplatform::joystick::guid(id).has_value());
		REQUIRE_FALSE(wplatform::joystick::state(id).has_value());
		REQUIRE_FALSE(wplatform::joystick::is_gamepad(id));
		REQUIRE_FALSE(wplatform::joystick::gamepad_name(id).has_value());
		REQUIRE_FALSE(wplatform::joystick::gamepad_state(id).has_value());
	}
}

TEST_CASE("invalid joystick ids are rejected", "[joystick]")
{
	PlatformGuard guard;

	const auto id{ static_cast<wplatform::joystick::Id>(wplatform::joystick::Count) };

	REQUIRE_FALSE(wplatform::joystick::present(id));
	REQUIRE_FALSE(wplatform::joystick::name(id).has_value());
	REQUIRE_FALSE(wplatform::joystick::state(id).has_value());
	REQUIRE_FALSE(wplatform::joystick::is_gamepad(id));
	REQUIRE_FALSE(wplatform::joystick::gamepad_state(id).has_value());
}

TEST_CASE("joystick state accessors are bounds checked", "[joystick]")
{
	wplatform::joystick::State state;

	REQUIRE(state.axis(0) == 0.0f);
	REQUIRE_FALSE(state.button(0));
	REQUIRE(state.hat(0) == wplatform::joystick::HatCentered);

	state.axes.push_back(0.5f);
	state.buttons.push_back(1);
	state.hats.push_back(wplatform::joystick::HatRightUp);

	REQUIRE(state.axis(0) == 0.5f);
	REQUIRE(state.button(0));
	REQUIRE(state.hat(0) == wplatform::joystick::HatRightUp);

	REQUIRE(state.axis(1) == 0.0f);
	REQUIRE_FALSE(state.button(1));
	REQUIRE(state.hat(1) == wplatform::joystick::HatCentered);
}

TEST_CASE("gamepad state defaults to neutral", "[joystick]")
{
	const wplatform::joystick::GamepadState state;

	REQUIRE_FALSE(state.down(wplatform::joystick::GamepadButton::a));
	REQUIRE(state.axis(wplatform::joystick::GamepadAxis::left_x) == 0.0f);

	REQUIRE_FALSE(state.down(static_cast<wplatform::joystick::GamepadButton>(99)));
	REQUIRE(state.axis(static_cast<wplatform::joystick::GamepadAxis>(99)) == 0.0f);
}

TEST_CASE("joystick events can be delivered to a window dispatcher", "[joystick][events]")
{
	PlatformGuard guard;

	wplatform::window::Config config;
	config.title = "wplatform joystick test";
	config.size = { 320, 240 };

	auto window{ wplatform::window::Window::create(config) };
	REQUIRE(window.has_value());

	bool connected{ false };
	bool disconnected{ false };
	wplatform::joystick::Id connectedId{ wplatform::joystick::Id::joy_1 };
	wplatform::joystick::Id disconnectedId{ wplatform::joystick::Id::joy_1 };

	window->dispatcher()->add_listener(wplatform::events::Type::joystick_connected,
		[&](const wplatform::events::Event& event)
		{
			const auto* data{ event.get<wplatform::events::JoystickConnected>() };
			if (data != nullptr)
			{
				connected = true;
				connectedId = data->id;
			}
		});

	window->dispatcher()->add_listener(wplatform::events::Type::joystick_disconnected,
		[&](const wplatform::events::Event& event)
		{
			const auto* data{ event.get<wplatform::events::JoystickDisconnected>() };
			if (data != nullptr)
			{
				disconnected = true;
				disconnectedId = data->id;
			}
		});

	window->dispatcher()->dispatch(wplatform::events::Event{
		wplatform::events::Type::joystick_connected,
		wplatform::events::JoystickConnected{ wplatform::joystick::Id::joy_2 } });

	window->dispatcher()->dispatch(wplatform::events::Event{
		wplatform::events::Type::joystick_disconnected,
		wplatform::events::JoystickDisconnected{ wplatform::joystick::Id::joy_2 } });

	REQUIRE(connected);
	REQUIRE(connectedId == wplatform::joystick::Id::joy_2);
	REQUIRE(disconnected);
	REQUIRE(disconnectedId == wplatform::joystick::Id::joy_2);
}
