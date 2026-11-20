#include <wplatform/joystick.hpp>

#include <wplatform/platform.hpp>

#include <wcore/log.hpp>

#include "internal.hpp"

#include <GLFW/glfw3.h>

#include <string>

namespace wplatform::joystick
{
	namespace
	{
		static_assert(Count == GLFW_JOYSTICK_LAST + 1);
		static_assert(static_cast<i32>(Id::joy_1) == GLFW_JOYSTICK_1);
		static_assert(static_cast<i32>(Id::joy_16) == GLFW_JOYSTICK_16);

		static_assert(GamepadButtonCount == GLFW_GAMEPAD_BUTTON_LAST + 1);
		static_assert(static_cast<i32>(GamepadButton::a) == GLFW_GAMEPAD_BUTTON_A);
		static_assert(static_cast<i32>(GamepadButton::b) == GLFW_GAMEPAD_BUTTON_B);
		static_assert(static_cast<i32>(GamepadButton::x) == GLFW_GAMEPAD_BUTTON_X);
		static_assert(static_cast<i32>(GamepadButton::y) == GLFW_GAMEPAD_BUTTON_Y);
		static_assert(static_cast<i32>(GamepadButton::left_bumper) == GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);
		static_assert(static_cast<i32>(GamepadButton::right_bumper) == GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);
		static_assert(static_cast<i32>(GamepadButton::back) == GLFW_GAMEPAD_BUTTON_BACK);
		static_assert(static_cast<i32>(GamepadButton::start) == GLFW_GAMEPAD_BUTTON_START);
		static_assert(static_cast<i32>(GamepadButton::guide) == GLFW_GAMEPAD_BUTTON_GUIDE);
		static_assert(static_cast<i32>(GamepadButton::left_thumb) == GLFW_GAMEPAD_BUTTON_LEFT_THUMB);
		static_assert(static_cast<i32>(GamepadButton::right_thumb) == GLFW_GAMEPAD_BUTTON_RIGHT_THUMB);
		static_assert(static_cast<i32>(GamepadButton::dpad_up) == GLFW_GAMEPAD_BUTTON_DPAD_UP);
		static_assert(static_cast<i32>(GamepadButton::dpad_right) == GLFW_GAMEPAD_BUTTON_DPAD_RIGHT);
		static_assert(static_cast<i32>(GamepadButton::dpad_down) == GLFW_GAMEPAD_BUTTON_DPAD_DOWN);
		static_assert(static_cast<i32>(GamepadButton::dpad_left) == GLFW_GAMEPAD_BUTTON_DPAD_LEFT);

		static_assert(GamepadAxisCount == GLFW_GAMEPAD_AXIS_LAST + 1);
		static_assert(static_cast<i32>(GamepadAxis::left_x) == GLFW_GAMEPAD_AXIS_LEFT_X);
		static_assert(static_cast<i32>(GamepadAxis::left_y) == GLFW_GAMEPAD_AXIS_LEFT_Y);
		static_assert(static_cast<i32>(GamepadAxis::right_x) == GLFW_GAMEPAD_AXIS_RIGHT_X);
		static_assert(static_cast<i32>(GamepadAxis::right_y) == GLFW_GAMEPAD_AXIS_RIGHT_Y);
		static_assert(static_cast<i32>(GamepadAxis::left_trigger) == GLFW_GAMEPAD_AXIS_LEFT_TRIGGER);
		static_assert(static_cast<i32>(GamepadAxis::right_trigger) == GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);

		static_assert(HatCentered == GLFW_HAT_CENTERED);
		static_assert(HatUp == GLFW_HAT_UP);
		static_assert(HatRight == GLFW_HAT_RIGHT);
		static_assert(HatDown == GLFW_HAT_DOWN);
		static_assert(HatLeft == GLFW_HAT_LEFT);
		static_assert(HatRightUp == GLFW_HAT_RIGHT_UP);
		static_assert(HatRightDown == GLFW_HAT_RIGHT_DOWN);
		static_assert(HatLeftUp == GLFW_HAT_LEFT_UP);
		static_assert(HatLeftDown == GLFW_HAT_LEFT_DOWN);

		bool can_query() noexcept
		{
			return platform::initialized() && platform::is_main_thread();
		}

		constexpr bool valid_id(Id id) noexcept
		{
			const i32 value{ static_cast<i32>(id) };
			return value >= 0 && static_cast<std::size_t>(value) < Count;
		}

		constexpr int to_glfw(Id id) noexcept
		{
			return static_cast<int>(id);
		}

		void on_joystick(int id, int event)
		{
			if (id < 0 || static_cast<std::size_t>(id) >= Count)
				return;

			const Id value{ static_cast<Id>(id) };

			if (event == GLFW_CONNECTED)
			{
				events::detail::broadcast(events::Event{
					events::Type::joystick_connected,
					events::JoystickConnected{ value } });
				return;
			}

			if (event == GLFW_DISCONNECTED)
			{
				events::detail::broadcast(events::Event{
					events::Type::joystick_disconnected,
					events::JoystickDisconnected{ value } });
			}
		}
	} // namespace

	f32 State::axis(std::size_t index) const noexcept
	{
		return index < axes.size() ? axes[index] : 0.0f;
	}

	bool State::button(std::size_t index) const noexcept
	{
		return index < buttons.size() && buttons[index] != 0;
	}

	u8 State::hat(std::size_t index) const noexcept
	{
		return index < hats.size() ? hats[index] : HatCentered;
	}

	bool GamepadState::down(GamepadButton button) const noexcept
	{
		const i32 value{ static_cast<i32>(button) };
		return value >= 0
			&& static_cast<std::size_t>(value) < buttons.size()
			&& buttons[static_cast<std::size_t>(value)] != 0;
	}

	f32 GamepadState::axis(GamepadAxis axis) const noexcept
	{
		const i32 value{ static_cast<i32>(axis) };
		return value >= 0 && static_cast<std::size_t>(value) < axes.size()
			? axes[static_cast<std::size_t>(value)]
			: 0.0f;
	}

	bool present(Id id) noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: joystick queries require an initialized platform on the main thread");
			return false;
		}

		if (!valid_id(id))
			return false;

		return glfwJoystickPresent(to_glfw(id)) == GLFW_TRUE;
	}

	std::optional<std::string> name(Id id)
	{
		if (!present(id))
			return std::nullopt;

		const char* value{ glfwGetJoystickName(to_glfw(id)) };
		if (value == nullptr)
			return std::nullopt;

		return std::string{ value };
	}

	std::optional<std::string> guid(Id id)
	{
		if (!present(id))
			return std::nullopt;

		const char* value{ glfwGetJoystickGUID(to_glfw(id)) };
		if (value == nullptr)
			return std::nullopt;

		return std::string{ value };
	}

	std::optional<State> state(Id id)
	{
		if (!present(id))
			return std::nullopt;

		int axisCount{ 0 };
		int buttonCount{ 0 };
		int hatCount{ 0 };

		const float* axes{ glfwGetJoystickAxes(to_glfw(id), &axisCount) };
		const unsigned char* buttons{ glfwGetJoystickButtons(to_glfw(id), &buttonCount) };
		const unsigned char* hats{ glfwGetJoystickHats(to_glfw(id), &hatCount) };

		if ((axisCount > 0 && axes == nullptr)
			|| (buttonCount > 0 && buttons == nullptr)
			|| (hatCount > 0 && hats == nullptr))
		{
			return std::nullopt;
		}

		State result;

		if (axisCount > 0)
			result.axes.assign(axes, axes + axisCount);

		if (buttonCount > 0)
			result.buttons.assign(buttons, buttons + buttonCount);

		if (hatCount > 0)
			result.hats.assign(hats, hats + hatCount);

		return result;
	}

	bool is_gamepad(Id id) noexcept
	{
		if (!present(id))
			return false;

		return glfwJoystickIsGamepad(to_glfw(id)) == GLFW_TRUE;
	}

	std::optional<std::string> gamepad_name(Id id)
	{
		if (!is_gamepad(id))
			return std::nullopt;

		const char* value{ glfwGetGamepadName(to_glfw(id)) };
		if (value == nullptr)
			return std::nullopt;

		return std::string{ value };
	}

	std::optional<GamepadState> gamepad_state(Id id)
	{
		if (!is_gamepad(id))
			return std::nullopt;

		GLFWgamepadstate native{};
		if (glfwGetGamepadState(to_glfw(id), &native) != GLFW_TRUE)
			return std::nullopt;

		GamepadState result;

		for (std::size_t i = 0; i < GamepadButtonCount; ++i)
			result.buttons[i] = native.buttons[i];

		for (std::size_t i = 0; i < GamepadAxisCount; ++i)
			result.axes[i] = native.axes[i];

		return result;
	}

	std::vector<Id> connected()
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: joystick queries require an initialized platform on the main thread");
			return {};
		}

		std::vector<Id> result;

		for (i32 i{ 0 }; i < static_cast<i32>(Count); ++i)
		{
			if (glfwJoystickPresent(i) == GLFW_TRUE)
				result.push_back(static_cast<Id>(i));
		}

		return result;
	}

	std::size_t connected_count() noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: joystick queries require an initialized platform on the main thread");
			return 0;
		}

		std::size_t result{ 0 };

		for (i32 i{ 0 }; i < static_cast<i32>(Count); ++i)
		{
			if (glfwJoystickPresent(i) == GLFW_TRUE)
				++result;
		}

		return result;
	}

	bool update_mappings(std::string_view mappings)
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: joystick queries require an initialized platform on the main thread");
			return false;
		}

		const std::string value{ mappings };
		return glfwUpdateGamepadMappings(value.c_str()) == GLFW_TRUE;
	}

	namespace detail
	{
		void install_callback() noexcept
		{
			glfwSetJoystickCallback(on_joystick);
			glfwJoystickPresent(GLFW_JOYSTICK_1);
		}
	} // namespace detail
} // namespace wplatform::joystick
