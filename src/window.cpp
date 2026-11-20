#include <wplatform/window.hpp>

#include <wplatform/events.hpp>
#include <wplatform/input.hpp>
#include <wplatform/platform.hpp>

#include <wcore/log.hpp>

#include "internal.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace wplatform::window
{
	namespace
	{
		static_assert(static_cast<i32>(events::Action::release) == GLFW_RELEASE);
		static_assert(static_cast<i32>(events::Action::press) == GLFW_PRESS);
		static_assert(static_cast<i32>(events::Action::repeat) == GLFW_REPEAT);

		static_assert(static_cast<i32>(events::Key::unknown) == GLFW_KEY_UNKNOWN);
		static_assert(static_cast<i32>(events::Key::space) == GLFW_KEY_SPACE);
		static_assert(static_cast<i32>(events::Key::a) == GLFW_KEY_A);
		static_assert(static_cast<i32>(events::Key::z) == GLFW_KEY_Z);
		static_assert(static_cast<i32>(events::Key::escape) == GLFW_KEY_ESCAPE);
		static_assert(static_cast<i32>(events::Key::f25) == GLFW_KEY_F25);
		static_assert(static_cast<i32>(events::Key::keypad_equal) == GLFW_KEY_KP_EQUAL);
		static_assert(static_cast<i32>(events::Key::menu) == GLFW_KEY_MENU);

		static_assert(static_cast<i32>(events::MouseButton::left) == GLFW_MOUSE_BUTTON_LEFT);
		static_assert(static_cast<i32>(events::MouseButton::right) == GLFW_MOUSE_BUTTON_RIGHT);
		static_assert(static_cast<i32>(events::MouseButton::middle) == GLFW_MOUSE_BUTTON_MIDDLE);
		static_assert(static_cast<i32>(events::MouseButton::button_8) == GLFW_MOUSE_BUTTON_8);

		Window* window_of(GLFWwindow* window) noexcept
		{
			return static_cast<Window*>(glfwGetWindowUserPointer(window));
		}

		void dispatch(GLFWwindow* window, events::Event event)
		{
			auto* target{ window_of(window) };
			if (target == nullptr)
				return;

			if (auto* input = target->input())
				input->handle(event);

			if (auto* dispatcher = target->dispatcher())
				dispatcher->dispatch(event);
		}

		events::Action to_action(int action) noexcept
		{
			switch (action)
			{
			case GLFW_RELEASE: return events::Action::release;
			case GLFW_PRESS: return events::Action::press;
			case GLFW_REPEAT: return events::Action::repeat;
			default: return events::Action::press;
			}
		}

		events::Modifiers to_modifiers(int mods) noexcept
		{
			return events::Modifiers{
				(mods & GLFW_MOD_SHIFT) != 0,
				(mods & GLFW_MOD_CONTROL) != 0,
				(mods & GLFW_MOD_ALT) != 0,
				(mods & GLFW_MOD_SUPER) != 0,
				(mods & GLFW_MOD_CAPS_LOCK) != 0,
				(mods & GLFW_MOD_NUM_LOCK) != 0,
			};
		}

		bool in_glfw_range(u32 value) noexcept
		{
			return value <= static_cast<u32>(std::numeric_limits<int>::max());
		}

		bool in_glfw_range(const std::optional<u32>& value) noexcept
		{
			return !value.has_value() || in_glfw_range(*value);
		}

		int to_glfw(const std::optional<u32>& value) noexcept
		{
			return value.has_value() ? static_cast<int>(*value) : GLFW_DONT_CARE;
		}

		void on_close(GLFWwindow* window)
		{
			dispatch(window, events::Event{ events::Type::close });
		}

		void on_focus(GLFWwindow* window, int focused)
		{
			dispatch(window, events::Event{
				events::Type::focus,
				events::FocusChanged{ focused == GLFW_TRUE } });
		}

		void on_iconify(GLFWwindow* window, int iconified)
		{
			dispatch(window, events::Event{
				events::Type::iconify,
				events::Iconified{ iconified == GLFW_TRUE } });
		}

		void on_maximize(GLFWwindow* window, int maximized)
		{
			dispatch(window, events::Event{
				events::Type::maximize,
				events::Maximized{ maximized == GLFW_TRUE } });
		}

		void on_resize(GLFWwindow* window, int width, int height)
		{
			dispatch(window, events::Event{
				events::Type::resize,
				events::Resized{ Extent{
					static_cast<u32>(width),
					static_cast<u32>(height) } } });
		}

		void on_framebuffer_resize(GLFWwindow* window, int width, int height)
		{
			dispatch(window, events::Event{
				events::Type::framebuffer_resize,
				events::FramebufferResized{ Extent{
					static_cast<u32>(width),
					static_cast<u32>(height) } } });
		}

		void on_move(GLFWwindow* window, int x, int y)
		{
			dispatch(window, events::Event{
				events::Type::move,
				events::Moved{ Position{
					static_cast<i32>(x),
					static_cast<i32>(y) } } });
		}

		void on_content_scale(GLFWwindow* window, float x, float y)
		{
			dispatch(window, events::Event{
				events::Type::content_scale,
				events::ContentScaleChanged{ Scale{ x, y } } });
		}

		void on_refresh(GLFWwindow* window)
		{
			dispatch(window, events::Event{ events::Type::refresh });
		}

		void on_key(GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			dispatch(window, events::Event{
				events::Type::key,
				events::KeyEvent{
					static_cast<events::Key>(key),
					static_cast<i32>(scancode),
					to_action(action),
					to_modifiers(mods) } });
		}

		void on_char(GLFWwindow* window, unsigned int codepoint)
		{
			dispatch(window, events::Event{
				events::Type::char_input,
				events::CharEvent{
					static_cast<u32>(codepoint),
					{} } });
		}

		void on_mouse_button(GLFWwindow* window, int button, int action, int mods)
		{
			dispatch(window, events::Event{
				events::Type::mouse_button,
				events::MouseButtonEvent{
					static_cast<events::MouseButton>(button),
					to_action(action),
					to_modifiers(mods) } });
		}

		void on_cursor_move(GLFWwindow* window, double x, double y)
		{
			dispatch(window, events::Event{
				events::Type::cursor_move,
				events::CursorMoved{ x, y } });
		}

		void on_cursor_enter(GLFWwindow* window, int entered)
		{
			dispatch(window, events::Event{
				events::Type::cursor_enter,
				events::CursorEntered{ entered == GLFW_TRUE } });
		}

		void on_scroll(GLFWwindow* window, double x, double y)
		{
			dispatch(window, events::Event{
				events::Type::scroll,
				events::Scrolled{ x, y } });
		}

		void on_drop(GLFWwindow* window, int count, const char** paths)
		{
			std::vector<std::string> values;
			values.reserve(static_cast<std::size_t>(count));

			for (int i{ 0 }; i < count; ++i)
			{
				if (paths != nullptr && paths[i] != nullptr)
					values.emplace_back(paths[i]);
			}

			dispatch(window, events::Event{
				events::Type::drop,
				events::FileDropped{ std::move(values) } });
		}

		void install_callbacks(GLFWwindow* window)
		{
			glfwSetWindowCloseCallback(window, on_close);
			glfwSetWindowFocusCallback(window, on_focus);
			glfwSetWindowIconifyCallback(window, on_iconify);
			glfwSetWindowMaximizeCallback(window, on_maximize);
			glfwSetWindowSizeCallback(window, on_resize);
			glfwSetFramebufferSizeCallback(window, on_framebuffer_resize);
			glfwSetWindowPosCallback(window, on_move);
			glfwSetWindowContentScaleCallback(window, on_content_scale);
			glfwSetWindowRefreshCallback(window, on_refresh);
			glfwSetKeyCallback(window, on_key);
			glfwSetCharCallback(window, on_char);
			glfwSetMouseButtonCallback(window, on_mouse_button);
			glfwSetCursorPosCallback(window, on_cursor_move);
			glfwSetCursorEnterCallback(window, on_cursor_enter);
			glfwSetScrollCallback(window, on_scroll);
			glfwSetDropCallback(window, on_drop);
		}
	} // namespace

	std::optional<Window> Window::create(const Config& config)
	{
		if (!platform::initialized())
		{
			WCORE_ERROR("wplatform: window creation requires platform::init()");
			return std::nullopt;
		}

		if (!platform::is_main_thread())
		{
			WCORE_ERROR("wplatform: window::create must be called from the main thread");
			return std::nullopt;
		}

		if (config.size.width == 0 || config.size.height == 0)
		{
			WCORE_ERROR("wplatform: window size must be non-zero ({}x{})",
				config.size.width, config.size.height);
			return std::nullopt;
		}

		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_DECORATED, config.decorated ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_FOCUSED, config.focused ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_MAXIMIZED, config.maximized ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_FLOATING, config.floating ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, config.autoIconify ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, config.mousePassthrough ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER,
			config.transparentFramebuffer ? GLFW_TRUE : GLFW_FALSE);

		GLFWwindow* handle{ glfwCreateWindow(
			static_cast<int>(config.size.width),
			static_cast<int>(config.size.height),
			config.title.c_str(),
			nullptr,
			nullptr) };

		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: failed to create window '{}' ({}x{})",
				config.title, config.size.width, config.size.height);
			return std::nullopt;
		}

		WCORE_INFO("wplatform: created window '{}' ({}x{})",
			config.title, config.size.width, config.size.height);

		auto dispatcher{ std::make_shared<events::Dispatcher>() };
		auto input{ std::make_unique<wplatform::input::State>() };

		events::detail::register_dispatcher(dispatcher);

		input->handle(events::Event{
			events::Type::focus,
			events::FocusChanged{
				glfwGetWindowAttrib(handle, GLFW_FOCUSED) == GLFW_TRUE } });

		double cursorX{ 0.0 };
		double cursorY{ 0.0 };
		glfwGetCursorPos(handle, &cursorX, &cursorY);
		input->handle(events::Event{
			events::Type::cursor_move,
			events::CursorMoved{ cursorX, cursorY } });

		install_callbacks(handle);

		Window result{ handle, std::move(dispatcher), std::move(input) };
		if (config.sizeLimits.any())
			result.set_size_limits(config.sizeLimits);

		return result;
	}

	Window::Window(GLFWwindow* handle,
		std::shared_ptr<events::Dispatcher> dispatcher,
		std::unique_ptr<wplatform::input::State> input) noexcept
		: mHandle{ handle }
		, mDispatcher{ std::move(dispatcher) }
		, mInput{ std::move(input) }
	{
		if (mHandle != nullptr)
			glfwSetWindowUserPointer(mHandle, this);
	}

	Window::Window(Window&& other) noexcept
		: mHandle{ std::exchange(other.mHandle, nullptr) }
		, mDispatcher{ std::move(other.mDispatcher) }
		, mInput{ std::move(other.mInput) }
	{
		if (mHandle != nullptr)
			glfwSetWindowUserPointer(mHandle, this);
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			mHandle = std::exchange(other.mHandle, nullptr);
			mDispatcher = std::move(other.mDispatcher);
			mInput = std::move(other.mInput);

			if (mHandle != nullptr)
				glfwSetWindowUserPointer(mHandle, this);
		}

		return *this;
	}

	Window::~Window()
	{
		destroy();
	}

	void Window::destroy() noexcept
	{
		if (mHandle == nullptr)
			return;

		if (platform::initialized())
			glfwDestroyWindow(mHandle);

		mHandle = nullptr;
	}

	bool Window::valid() const noexcept
	{
		return mHandle != nullptr;
	}

	GLFWwindow* Window::native_handle() const noexcept
	{
		return mHandle;
	}

	events::Dispatcher* Window::dispatcher() noexcept
	{
		return mDispatcher.get();
	}

	const events::Dispatcher* Window::dispatcher() const noexcept
	{
		return mDispatcher.get();
	}

	wplatform::input::State* Window::input() noexcept
	{
		return mInput.get();
	}

	const wplatform::input::State* Window::input() const noexcept
	{
		return mInput.get();
	}

	bool Window::should_close() const noexcept
	{
		return mHandle != nullptr && glfwWindowShouldClose(mHandle) == GLFW_TRUE;
	}

	void Window::request_close() noexcept
	{
		if (mHandle != nullptr)
			glfwSetWindowShouldClose(mHandle, GLFW_TRUE);
	}

	Extent Window::size() const noexcept
	{
		if (mHandle == nullptr)
			return {};

		int width{ 0 };
		int height{ 0 };
		glfwGetWindowSize(mHandle, &width, &height);

		return Extent{
			static_cast<u32>(width),
			static_cast<u32>(height),
		};
	}

	Extent Window::framebuffer_size() const noexcept
	{
		if (mHandle == nullptr)
			return {};

		int width{ 0 };
		int height{ 0 };
		glfwGetFramebufferSize(mHandle, &width, &height);

		return Extent{
			static_cast<u32>(width),
			static_cast<u32>(height),
		};
	}

	void Window::set_size(u32 width, u32 height) noexcept
	{
		if (mHandle != nullptr)
		{
			glfwSetWindowSize(mHandle,
				static_cast<int>(width),
				static_cast<int>(height));
		}
	}

	void Window::set_size_limits(const SizeLimits& limits) noexcept
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: setting size limits requires a valid window");
			return;
		}

		if (!in_glfw_range(limits.minWidth) || !in_glfw_range(limits.minHeight)
			|| !in_glfw_range(limits.maxWidth) || !in_glfw_range(limits.maxHeight))
		{
			WCORE_ERROR("wplatform: window size limits are out of range");
			return;
		}

		glfwSetWindowSizeLimits(mHandle,
			to_glfw(limits.minWidth),
			to_glfw(limits.minHeight),
			to_glfw(limits.maxWidth),
			to_glfw(limits.maxHeight));
	}

	void Window::clear_size_limits() noexcept
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: clearing size limits requires a valid window");
			return;
		}

		glfwSetWindowSizeLimits(mHandle,
			GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE, GLFW_DONT_CARE);
	}

	void Window::set_aspect_ratio(u32 numerator, u32 denominator) noexcept
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: setting an aspect ratio requires a valid window");
			return;
		}

		if (numerator == 0 || denominator == 0
			|| !in_glfw_range(numerator) || !in_glfw_range(denominator))
		{
			WCORE_ERROR("wplatform: invalid window aspect ratio {}x{}",
				numerator, denominator);
			return;
		}

		glfwSetWindowAspectRatio(mHandle,
			static_cast<int>(numerator), static_cast<int>(denominator));
	}

	void Window::clear_aspect_ratio() noexcept
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: clearing an aspect ratio requires a valid window");
			return;
		}

		glfwSetWindowAspectRatio(mHandle, GLFW_DONT_CARE, GLFW_DONT_CARE);
	}

	Position Window::position() const noexcept
	{
		if (mHandle == nullptr)
			return {};

		int x{ 0 };
		int y{ 0 };
		glfwGetWindowPos(mHandle, &x, &y);

		return Position{
			static_cast<i32>(x),
			static_cast<i32>(y),
		};
	}

	void Window::set_position(i32 x, i32 y) noexcept
	{
		if (mHandle != nullptr)
		{
			glfwSetWindowPos(mHandle,
				static_cast<int>(x),
				static_cast<int>(y));
		}
	}

	Scale Window::content_scale() const noexcept
	{
		if (mHandle == nullptr)
			return { 1.0f, 1.0f };

		float x{ 1.0f };
		float y{ 1.0f };
		glfwGetWindowContentScale(mHandle, &x, &y);

		return Scale{ x, y };
	}

	std::string_view Window::title() const noexcept
	{
		if (mHandle == nullptr)
			return {};

		const char* value = glfwGetWindowTitle(mHandle);
		return value != nullptr ? value : "";
	}

	void Window::set_title(std::string_view title)
	{
		if (mHandle == nullptr)
			return;

		const std::string value{ title };
		glfwSetWindowTitle(mHandle, value.c_str());
	}

	bool Window::set_icons(std::span<const Icon> icons) noexcept
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: setting window icons requires a valid window");
			return false;
		}

		std::vector<std::vector<u8>> pixels;
		pixels.reserve(icons.size());

		for (const Icon& icon : icons)
		{
			if (!icon.valid())
			{
				WCORE_ERROR("wplatform: window icon image does not match its dimensions");
				return false;
			}

			pixels.push_back(icon.pixels);
		}

		std::vector<GLFWimage> native;
		native.reserve(icons.size());

		for (std::size_t i{ 0 }; i < icons.size(); ++i)
		{
			native.push_back(GLFWimage{
				static_cast<int>(icons[i].size.width),
				static_cast<int>(icons[i].size.height),
				pixels[i].data() });
		}

		glfwSetWindowIcon(mHandle,
			static_cast<int>(native.size()),
			native.empty() ? nullptr : native.data());
		return true;
	}

	bool Window::set_icon(const Icon& icon) noexcept
	{
		return set_icons(std::span<const Icon>{ &icon, 1 });
	}

	f32 Window::opacity() const noexcept
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: querying opacity requires a valid window");
			return 1.0f;
		}

		return glfwGetWindowOpacity(mHandle);
	}

	void Window::set_opacity(f32 opacity) noexcept
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: setting opacity requires a valid window");
			return;
		}

		if (!std::isfinite(opacity) || opacity < 0.0f || opacity > 1.0f)
		{
			WCORE_ERROR("wplatform: window opacity must be between 0 and 1 ({})", opacity);
			return;
		}

		glfwSetWindowOpacity(mHandle, opacity);
	}

	void Window::request_attention() noexcept
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: requesting attention requires a valid window");
			return;
		}

		glfwRequestWindowAttention(mHandle);
	}

	void Window::show() noexcept
	{
		if (mHandle != nullptr)
			glfwShowWindow(mHandle);
	}

	void Window::hide() noexcept
	{
		if (mHandle != nullptr)
			glfwHideWindow(mHandle);
	}

	void Window::focus() noexcept
	{
		if (mHandle != nullptr)
			glfwFocusWindow(mHandle);
	}

	void Window::iconify() noexcept
	{
		if (mHandle != nullptr)
			glfwIconifyWindow(mHandle);
	}

	void Window::restore() noexcept
	{
		if (mHandle != nullptr)
			glfwRestoreWindow(mHandle);
	}

	void Window::maximize() noexcept
	{
		if (mHandle != nullptr)
			glfwMaximizeWindow(mHandle);
	}

	bool Window::visible() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_VISIBLE) == GLFW_TRUE;
	}

	bool Window::focused() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_FOCUSED) == GLFW_TRUE;
	}

	bool Window::iconified() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_ICONIFIED) == GLFW_TRUE;
	}

	bool Window::maximized() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_MAXIMIZED) == GLFW_TRUE;
	}

	bool Window::resizable() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_RESIZABLE) == GLFW_TRUE;
	}

	void Window::set_resizable(bool resizable) noexcept
	{
		if (mHandle != nullptr)
		{
			glfwSetWindowAttrib(mHandle, GLFW_RESIZABLE,
				resizable ? GLFW_TRUE : GLFW_FALSE);
		}
	}

	bool Window::hovered() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_HOVERED) == GLFW_TRUE;
	}

	bool Window::transparent_framebuffer() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_TRANSPARENT_FRAMEBUFFER) == GLFW_TRUE;
	}

	bool Window::floating() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_FLOATING) == GLFW_TRUE;
	}

	void Window::set_floating(bool enabled) noexcept
	{
		if (mHandle != nullptr)
		{
			glfwSetWindowAttrib(mHandle, GLFW_FLOATING,
				enabled ? GLFW_TRUE : GLFW_FALSE);
		}
	}

	bool Window::auto_iconify() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_AUTO_ICONIFY) == GLFW_TRUE;
	}

	void Window::set_auto_iconify(bool enabled) noexcept
	{
		if (mHandle != nullptr)
		{
			glfwSetWindowAttrib(mHandle, GLFW_AUTO_ICONIFY,
				enabled ? GLFW_TRUE : GLFW_FALSE);
		}
	}

	bool Window::mouse_passthrough() const noexcept
	{
		return mHandle != nullptr
			&& glfwGetWindowAttrib(mHandle, GLFW_MOUSE_PASSTHROUGH) == GLFW_TRUE;
	}

	void Window::set_mouse_passthrough(bool enabled) noexcept
	{
		if (mHandle != nullptr)
		{
			glfwSetWindowAttrib(mHandle, GLFW_MOUSE_PASSTHROUGH,
				enabled ? GLFW_TRUE : GLFW_FALSE);
		}
	}

	std::optional<monitor::Monitor> Window::fullscreen_monitor() const
	{
		if (mHandle == nullptr)
		{
			WCORE_ERROR("wplatform: querying the fullscreen monitor requires a valid window");
			return std::nullopt;
		}

		GLFWmonitor* handle{ glfwGetWindowMonitor(mHandle) };
		if (handle == nullptr)
			return std::nullopt;

		return std::optional<monitor::Monitor>{ monitor::Monitor{ handle } };
	}

	bool Window::set_fullscreen(const monitor::Monitor& target,
		const monitor::VideoMode& videoMode) noexcept
	{
		if (mHandle == nullptr || !target.valid()
			|| videoMode.size.width == 0 || videoMode.size.height == 0
			|| !in_glfw_range(videoMode.size.width)
			|| !in_glfw_range(videoMode.size.height))
		{
			WCORE_ERROR("wplatform: entering fullscreen requires a valid window, monitor and video mode");
			return false;
		}

		const int refreshRate{ videoMode.refreshRate > 0
			&& in_glfw_range(videoMode.refreshRate)
			? static_cast<int>(videoMode.refreshRate)
			: GLFW_DONT_CARE };

		glfwSetWindowMonitor(mHandle,
			target.native_handle(),
			0, 0,
			static_cast<int>(videoMode.size.width),
			static_cast<int>(videoMode.size.height),
			refreshRate);
		return true;
	}

	bool Window::set_windowed(Position position, Extent size) noexcept
	{
		if (mHandle == nullptr || size.width == 0 || size.height == 0
			|| !in_glfw_range(size.width) || !in_glfw_range(size.height))
		{
			WCORE_ERROR("wplatform: leaving fullscreen requires a valid window and non-zero size");
			return false;
		}

		glfwSetWindowMonitor(mHandle,
			nullptr,
			position.x, position.y,
			static_cast<int>(size.width),
			static_cast<int>(size.height),
			GLFW_DONT_CARE);
		return true;
	}
} // namespace wplatform::window
