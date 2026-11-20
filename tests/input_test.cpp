#include <wplatform/input.hpp>
#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <catch2/catch_test_macros.hpp>

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
		config.title = "wplatform input test";
		config.size = { 640, 480 };
		return config;
	}

	wplatform::events::Event key_event(wplatform::events::Key key,
		wplatform::events::Action action)
	{
		return wplatform::events::Event{
			wplatform::events::Type::key,
			wplatform::events::KeyEvent{ key, 0, action, {} } };
	}

	wplatform::events::Event mouse_event(wplatform::events::MouseButton button,
		wplatform::events::Action action)
	{
		return wplatform::events::Event{
			wplatform::events::Type::mouse_button,
			wplatform::events::MouseButtonEvent{ button, action, {} } };
	}
} // namespace

TEST_CASE("keys track down, pressed, and released across frames", "[input]")
{
	wplatform::input::State state;

	REQUIRE_FALSE(state.key_down(wplatform::events::Key::a));
	REQUIRE_FALSE(state.key_pressed(wplatform::events::Key::a));

	state.handle(key_event(wplatform::events::Key::a, wplatform::events::Action::press));

	REQUIRE(state.key_down(wplatform::events::Key::a));
	REQUIRE(state.key_pressed(wplatform::events::Key::a));
	REQUIRE_FALSE(state.key_released(wplatform::events::Key::a));

	state.new_frame();

	REQUIRE(state.key_down(wplatform::events::Key::a));
	REQUIRE_FALSE(state.key_pressed(wplatform::events::Key::a));

	state.handle(key_event(wplatform::events::Key::a, wplatform::events::Action::release));

	REQUIRE_FALSE(state.key_down(wplatform::events::Key::a));
	REQUIRE(state.key_released(wplatform::events::Key::a));

	state.new_frame();

	REQUIRE_FALSE(state.key_released(wplatform::events::Key::a));
}

TEST_CASE("key repeats keep the key down without counting as presses", "[input]")
{
	wplatform::input::State state;

	state.handle(key_event(wplatform::events::Key::space, wplatform::events::Action::press));
	state.new_frame();

	state.handle(key_event(wplatform::events::Key::space, wplatform::events::Action::repeat));

	REQUIRE(state.key_down(wplatform::events::Key::space));
	REQUIRE_FALSE(state.key_pressed(wplatform::events::Key::space));
}

TEST_CASE("mouse buttons track down, pressed, and released", "[input]")
{
	wplatform::input::State state;

	state.handle(mouse_event(
		wplatform::events::MouseButton::left, wplatform::events::Action::press));

	REQUIRE(state.mouse_down(wplatform::events::MouseButton::left));
	REQUIRE(state.mouse_pressed(wplatform::events::MouseButton::left));
	REQUIRE_FALSE(state.mouse_down(wplatform::events::MouseButton::right));

	state.new_frame();

	REQUIRE(state.mouse_down(wplatform::events::MouseButton::left));
	REQUIRE_FALSE(state.mouse_pressed(wplatform::events::MouseButton::left));

	state.handle(mouse_event(
		wplatform::events::MouseButton::left, wplatform::events::Action::release));

	REQUIRE_FALSE(state.mouse_down(wplatform::events::MouseButton::left));
	REQUIRE(state.mouse_released(wplatform::events::MouseButton::left));
}

TEST_CASE("cursor and scroll deltas accumulate", "[input]")
{
	wplatform::input::State state;

	state.handle(wplatform::events::Event{
		wplatform::events::Type::cursor_move,
		wplatform::events::CursorMoved{ 12.5, -3.0 } });

	REQUIRE(state.cursor().x == 12.5);
	REQUIRE(state.cursor().y == -3.0);

	state.handle(wplatform::events::Event{
		wplatform::events::Type::scroll,
		wplatform::events::Scrolled{ 0.0, 1.0 } });
	state.handle(wplatform::events::Event{
		wplatform::events::Type::scroll,
		wplatform::events::Scrolled{ 2.0, 2.5 } });

	REQUIRE(state.scroll_x() == 2.0);
	REQUIRE(state.scroll_y() == 3.5);

	state.new_frame();

	REQUIRE(state.scroll_x() == 0.0);
	REQUIRE(state.scroll_y() == 0.0);
	REQUIRE(state.cursor().x == 12.5);
}

TEST_CASE("focus and cursor enter are tracked", "[input]")
{
	wplatform::input::State state;

	REQUIRE_FALSE(state.focused());
	REQUIRE_FALSE(state.cursor_inside());

	state.handle(wplatform::events::Event{
		wplatform::events::Type::focus,
		wplatform::events::FocusChanged{ true } });
	state.handle(wplatform::events::Event{
		wplatform::events::Type::cursor_enter,
		wplatform::events::CursorEntered{ true } });

	REQUIRE(state.focused());
	REQUIRE(state.cursor_inside());

	state.handle(wplatform::events::Event{
		wplatform::events::Type::cursor_enter,
		wplatform::events::CursorEntered{ false } });

	REQUIRE_FALSE(state.cursor_inside());
}

TEST_CASE("invalid keys and buttons are ignored", "[input]")
{
	wplatform::input::State state;

	const auto invalidKey = static_cast<wplatform::events::Key>(10000);
	const auto invalidButton = static_cast<wplatform::events::MouseButton>(10000);

	state.handle(key_event(invalidKey, wplatform::events::Action::press));
	state.handle(mouse_event(invalidButton, wplatform::events::Action::press));

	REQUIRE_FALSE(state.key_down(wplatform::events::Key::unknown));
	REQUIRE_FALSE(state.key_down(invalidKey));
	REQUIRE_FALSE(state.mouse_down(invalidButton));
}

TEST_CASE("reset clears all input state", "[input]")
{
	wplatform::input::State state;

	state.handle(key_event(wplatform::events::Key::a, wplatform::events::Action::press));
	state.handle(mouse_event(
		wplatform::events::MouseButton::left, wplatform::events::Action::press));
	state.handle(wplatform::events::Event{
		wplatform::events::Type::cursor_move,
		wplatform::events::CursorMoved{ 5.0, 6.0 } });
	state.handle(wplatform::events::Event{
		wplatform::events::Type::scroll,
		wplatform::events::Scrolled{ 1.0, 2.0 } });
	state.handle(wplatform::events::Event{
		wplatform::events::Type::focus,
		wplatform::events::FocusChanged{ true } });

	state.reset();

	REQUIRE_FALSE(state.key_down(wplatform::events::Key::a));
	REQUIRE_FALSE(state.key_pressed(wplatform::events::Key::a));
	REQUIRE_FALSE(state.mouse_down(wplatform::events::MouseButton::left));
	REQUIRE(state.cursor().x == 0.0);
	REQUIRE(state.cursor().y == 0.0);
	REQUIRE(state.scroll_x() == 0.0);
	REQUIRE(state.scroll_y() == 0.0);
	REQUIRE_FALSE(state.focused());
}

TEST_CASE("windows own an input state", "[input][window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());
	REQUIRE(window->input() != nullptr);
}

TEST_CASE("window input state follows window events", "[input][window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());

	window->focus();
	REQUIRE(window->input()->focused());
}

TEST_CASE("window input state survives moves", "[input][window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());

	auto* input = window->input();

	wplatform::window::Window moved{ std::move(*window) };
	REQUIRE(moved.input() == input);
	REQUIRE(window->input() == nullptr);

	moved.focus();
	REQUIRE(moved.input()->focused());
}
