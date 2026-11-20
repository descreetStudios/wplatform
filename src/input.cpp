#include <wplatform/input.hpp>

namespace wplatform::input
{
	namespace
	{
		constexpr bool valid(events::Key key) noexcept
		{
			const i32 value{ static_cast<i32>(key) };
			return value >= 0 && static_cast<std::size_t>(value) < State::KeyCount;
		}

		constexpr bool valid(events::MouseButton button) noexcept
		{
			const i32 value{ static_cast<i32>(button) };
			return value >= 0 && static_cast<std::size_t>(value) < State::MouseButtonCount;
		}

		constexpr std::size_t index(events::Key key) noexcept
		{
			return static_cast<std::size_t>(key);
		}

		constexpr std::size_t index(events::MouseButton button) noexcept
		{
			return static_cast<std::size_t>(button);
		}
	} // namespace

	void State::handle(const events::Event& event)
	{
		switch (event.type)
		{
		case events::Type::key:
			if (const auto* key = event.get<events::KeyEvent>();
				key != nullptr && valid(key->key))
			{
				const std::size_t position{ index(key->key) };
				switch (key->action)
				{
				case events::Action::press:
					mKeysDown.set(position);
					mKeysPressed.set(position);
					break;
				case events::Action::release:
					mKeysDown.reset(position);
					mKeysReleased.set(position);
					break;
				case events::Action::repeat:
					mKeysDown.set(position);
					break;
				}
			}
			break;

		case events::Type::mouse_button:
			if (const auto* button = event.get<events::MouseButtonEvent>();
				button != nullptr && valid(button->button))
			{
				const std::size_t position{ index(button->button) };
				switch (button->action)
				{
				case events::Action::press:
					mButtonsDown.set(position);
					mButtonsPressed.set(position);
					break;
				case events::Action::release:
					mButtonsDown.reset(position);
					mButtonsReleased.set(position);
					break;
				case events::Action::repeat:
					break;
				}
			}
			break;

		case events::Type::cursor_move:
			if (const auto* moved = event.get<events::CursorMoved>())
				mCursor = Cursor{ moved->x, moved->y };
			break;

		case events::Type::cursor_enter:
			if (const auto* entered = event.get<events::CursorEntered>())
				mCursorInside = entered->entered;
			break;

		case events::Type::scroll:
			if (const auto* scrolled = event.get<events::Scrolled>())
			{
				mScrollX += scrolled->x;
				mScrollY += scrolled->y;
			}
			break;

		case events::Type::focus:
			if (const auto* focus = event.get<events::FocusChanged>())
				mFocused = focus->focused;
			break;

		default:
			break;
		}
	}

	void State::new_frame() noexcept
	{
		mKeysPressed.reset();
		mKeysReleased.reset();
		mButtonsPressed.reset();
		mButtonsReleased.reset();
		mScrollX = 0.0;
		mScrollY = 0.0;
	}

	void State::reset() noexcept
	{
		mKeysDown.reset();
		mKeysPressed.reset();
		mKeysReleased.reset();
		mButtonsDown.reset();
		mButtonsPressed.reset();
		mButtonsReleased.reset();

		mCursor = {};
		mScrollX = 0.0;
		mScrollY = 0.0;
		mFocused = false;
		mCursorInside = false;
	}

	bool State::key_down(events::Key key) const noexcept
	{
		return valid(key) && mKeysDown.test(index(key));
	}

	bool State::key_pressed(events::Key key) const noexcept
	{
		return valid(key) && mKeysPressed.test(index(key));
	}

	bool State::key_released(events::Key key) const noexcept
	{
		return valid(key) && mKeysReleased.test(index(key));
	}

	bool State::mouse_down(events::MouseButton button) const noexcept
	{
		return valid(button) && mButtonsDown.test(index(button));
	}

	bool State::mouse_pressed(events::MouseButton button) const noexcept
	{
		return valid(button) && mButtonsPressed.test(index(button));
	}

	bool State::mouse_released(events::MouseButton button) const noexcept
	{
		return valid(button) && mButtonsReleased.test(index(button));
	}

	Cursor State::cursor() const noexcept
	{
		return mCursor;
	}

	f64 State::scroll_x() const noexcept
	{
		return mScrollX;
	}

	f64 State::scroll_y() const noexcept
	{
		return mScrollY;
	}

	bool State::focused() const noexcept
	{
		return mFocused;
	}

	bool State::cursor_inside() const noexcept
	{
		return mCursorInside;
	}
} // namespace wplatform::input
