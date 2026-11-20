#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <wcore/types.hpp>

#include <wplatform/monitor.hpp>
#include <wplatform/types.hpp>

namespace wplatform::events
{
	enum class Type
	{
		close,
		focus,
		iconify,
		maximize,
		resize,
		framebuffer_resize,
		move,
		content_scale,
		refresh,
		key,
		char_input,
		mouse_button,
		cursor_move,
		cursor_enter,
		scroll,
		drop,
		monitor_connected,
		monitor_disconnected,
	};

	enum class Action
	{
		release,
		press,
		repeat,
	};

	enum class Key : i32
	{
		unknown = -1,

		space = 32,
		apostrophe = 39,
		comma = 44,
		minus = 45,
		period = 46,
		slash = 47,

		digit_0 = 48,
		digit_1 = 49,
		digit_2 = 50,
		digit_3 = 51,
		digit_4 = 52,
		digit_5 = 53,
		digit_6 = 54,
		digit_7 = 55,
		digit_8 = 56,
		digit_9 = 57,

		semicolon = 59,
		equal = 61,

		a = 65,
		b = 66,
		c = 67,
		d = 68,
		e = 69,
		f = 70,
		g = 71,
		h = 72,
		i = 73,
		j = 74,
		k = 75,
		l = 76,
		m = 77,
		n = 78,
		o = 79,
		p = 80,
		q = 81,
		r = 82,
		s = 83,
		t = 84,
		u = 85,
		v = 86,
		w = 87,
		x = 88,
		y = 89,
		z = 90,

		left_bracket = 91,
		backslash = 92,
		right_bracket = 93,
		grave_accent = 96,

		world_1 = 161,
		world_2 = 162,

		escape = 256,
		enter = 257,
		tab = 258,
		backspace = 259,
		insert = 260,
		del = 261,
		right = 262,
		left = 263,
		down = 264,
		up = 265,
		page_up = 266,
		page_down = 267,
		home = 268,
		end = 269,

		caps_lock = 280,
		scroll_lock = 281,
		num_lock = 282,
		print_screen = 283,
		pause = 284,

		f1 = 290,
		f2 = 291,
		f3 = 292,
		f4 = 293,
		f5 = 294,
		f6 = 295,
		f7 = 296,
		f8 = 297,
		f9 = 298,
		f10 = 299,
		f11 = 300,
		f12 = 301,
		f13 = 302,
		f14 = 303,
		f15 = 304,
		f16 = 305,
		f17 = 306,
		f18 = 307,
		f19 = 308,
		f20 = 309,
		f21 = 310,
		f22 = 311,
		f23 = 312,
		f24 = 313,
		f25 = 314,

		keypad_0 = 320,
		keypad_1 = 321,
		keypad_2 = 322,
		keypad_3 = 323,
		keypad_4 = 324,
		keypad_5 = 325,
		keypad_6 = 326,
		keypad_7 = 327,
		keypad_8 = 328,
		keypad_9 = 329,
		keypad_decimal = 330,
		keypad_divide = 331,
		keypad_multiply = 332,
		keypad_subtract = 333,
		keypad_add = 334,
		keypad_enter = 335,
		keypad_equal = 336,

		left_shift = 340,
		left_control = 341,
		left_alt = 342,
		left_super = 343,
		right_shift = 344,
		right_control = 345,
		right_alt = 346,
		right_super = 347,
		menu = 348,
	};

	enum class MouseButton : i32
	{
		left = 0,
		right = 1,
		middle = 2,
		button_4 = 3,
		button_5 = 4,
		button_6 = 5,
		button_7 = 6,
		button_8 = 7,
	};

	struct Modifiers
	{
		bool shift{ false };
		bool control{ false };
		bool alt{ false };
		bool super{ false };
		bool capsLock{ false };
		bool numLock{ false };
	};

	struct FocusChanged
	{
		bool focused;
	};

	struct Iconified
	{
		bool iconified;
	};

	struct Maximized
	{
		bool maximized;
	};

	struct Resized
	{
		Extent size;
	};

	struct FramebufferResized
	{
		Extent size;
	};

	struct Moved
	{
		Position position;
	};

	struct ContentScaleChanged
	{
		Scale scale;
	};

	struct Refreshed
	{
	};

	struct KeyEvent
	{
		Key key;
		i32 scancode;
		Action action;
		Modifiers modifiers;
	};

	struct CharEvent
	{
		u32 codepoint;
		Modifiers modifiers;
	};

	struct MouseButtonEvent
	{
		MouseButton button;
		Action action;
		Modifiers modifiers;
	};

	struct CursorMoved
	{
		f64 x;
		f64 y;
	};

	struct CursorEntered
	{
		bool entered;
	};

	struct Scrolled
	{
		f64 x;
		f64 y;
	};

	struct FileDropped
	{
		std::vector<std::string> paths;
	};

	struct MonitorConnected
	{
		monitor::Monitor handle;
	};

	struct MonitorDisconnected
	{
		monitor::Monitor handle;
		std::string name;
	};

	using Payload = std::variant<
		std::monostate,
		FocusChanged,
		Iconified,
		Maximized,
		Resized,
		FramebufferResized,
		Moved,
		ContentScaleChanged,
		Refreshed,
		KeyEvent,
		CharEvent,
		MouseButtonEvent,
		CursorMoved,
		CursorEntered,
		Scrolled,
		FileDropped,
		MonitorConnected,
		MonitorDisconnected>;

	struct Event
	{
		Type type;
		Payload payload{};

		bool is(Type expected) const noexcept
		{
			return type == expected;
		}

		template <typename T>
		const T* get() const noexcept
		{
			return std::get_if<T>(&payload);
		}

		template <typename T>
		T* get() noexcept
		{
			return std::get_if<T>(&payload);
		}
	};

	class Dispatcher
	{
	public:
		using Callback = std::function<void(const Event&)>;
		using ListenerId = std::uint64_t;

		static constexpr ListenerId InvalidListener{ 0 };

		Dispatcher();
		~Dispatcher();

		Dispatcher(Dispatcher&&) noexcept;
		Dispatcher& operator=(Dispatcher&&) noexcept;

		Dispatcher(const Dispatcher&) = delete;
		Dispatcher& operator=(const Dispatcher&) = delete;

		ListenerId add_listener(Type type, Callback callback);
		ListenerId add_listener(Callback callback);
		bool remove_listener(ListenerId id);
		void clear();

		void dispatch(const Event& event);

		bool has_listeners(Type type) const noexcept;
		std::size_t listener_count() const noexcept;

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};

	bool poll() noexcept;
	bool wait() noexcept;
	bool wait(f64 timeout) noexcept;
} // namespace wplatform::events
