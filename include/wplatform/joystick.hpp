#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <wcore/types.hpp>

namespace wplatform::joystick
{
	inline constexpr std::size_t Count{ 16 };
	inline constexpr std::size_t GamepadButtonCount{ 15 };
	inline constexpr std::size_t GamepadAxisCount{ 6 };

	inline constexpr u8 HatCentered{ 0 };
	inline constexpr u8 HatUp{ 1 };
	inline constexpr u8 HatRight{ 2 };
	inline constexpr u8 HatDown{ 4 };
	inline constexpr u8 HatLeft{ 8 };
	inline constexpr u8 HatRightUp{ HatRight | HatUp };
	inline constexpr u8 HatRightDown{ HatRight | HatDown };
	inline constexpr u8 HatLeftUp{ HatLeft | HatUp };
	inline constexpr u8 HatLeftDown{ HatLeft | HatDown };

	enum class Id : i32
	{
		joy_1 = 0,
		joy_2,
		joy_3,
		joy_4,
		joy_5,
		joy_6,
		joy_7,
		joy_8,
		joy_9,
		joy_10,
		joy_11,
		joy_12,
		joy_13,
		joy_14,
		joy_15,
		joy_16,
	};

	enum class GamepadButton : i32
	{
		a = 0,
		b,
		x,
		y,
		left_bumper,
		right_bumper,
		back,
		start,
		guide,
		left_thumb,
		right_thumb,
		dpad_up,
		dpad_right,
		dpad_down,
		dpad_left,
	};

	enum class GamepadAxis : i32
	{
		left_x = 0,
		left_y,
		right_x,
		right_y,
		left_trigger,
		right_trigger,
	};

	struct State
	{
		std::vector<f32> axes;
		std::vector<u8> buttons;
		std::vector<u8> hats;

		f32 axis(std::size_t index) const noexcept;
		bool button(std::size_t index) const noexcept;
		u8 hat(std::size_t index) const noexcept;
	};

	struct GamepadState
	{
		std::array<u8, GamepadButtonCount> buttons{};
		std::array<f32, GamepadAxisCount> axes{};

		bool down(GamepadButton button) const noexcept;
		f32 axis(GamepadAxis axis) const noexcept;
	};

	bool present(Id id) noexcept;

	std::optional<std::string> name(Id id);
	std::optional<std::string> guid(Id id);

	std::optional<State> state(Id id);

	bool is_gamepad(Id id) noexcept;
	std::optional<std::string> gamepad_name(Id id);
	std::optional<GamepadState> gamepad_state(Id id);

	std::vector<Id> connected();
	std::size_t connected_count() noexcept;

	bool update_mappings(std::string_view mappings);
} // namespace wplatform::joystick
