#pragma once

#include <bitset>
#include <cstddef>

#include <wcore/types.hpp>

#include <wplatform/events.hpp>

namespace wplatform::input
{
	struct Cursor
	{
		f64 x{ 0.0 };
		f64 y{ 0.0 };
	};

	class State
	{
	public:
		static constexpr std::size_t KeyCount{
			static_cast<std::size_t>(events::Key::menu) + 1 };
		static constexpr std::size_t MouseButtonCount{
			static_cast<std::size_t>(events::MouseButton::button_8) + 1 };

		void handle(const events::Event& event);
		void new_frame() noexcept;
		void reset() noexcept;

		bool key_down(events::Key key) const noexcept;
		bool key_pressed(events::Key key) const noexcept;
		bool key_released(events::Key key) const noexcept;

		bool mouse_down(events::MouseButton button) const noexcept;
		bool mouse_pressed(events::MouseButton button) const noexcept;
		bool mouse_released(events::MouseButton button) const noexcept;

		Cursor cursor() const noexcept;
		f64 scroll_x() const noexcept;
		f64 scroll_y() const noexcept;

		bool focused() const noexcept;
		bool cursor_inside() const noexcept;

	private:
		std::bitset<KeyCount> mKeysDown;
		std::bitset<KeyCount> mKeysPressed;
		std::bitset<KeyCount> mKeysReleased;

		std::bitset<MouseButtonCount> mButtonsDown;
		std::bitset<MouseButtonCount> mButtonsPressed;
		std::bitset<MouseButtonCount> mButtonsReleased;

		Cursor mCursor{};
		f64 mScrollX{ 0.0 };
		f64 mScrollY{ 0.0 };

		bool mFocused{ false };
		bool mCursorInside{ false };
	};
} // namespace wplatform::input
