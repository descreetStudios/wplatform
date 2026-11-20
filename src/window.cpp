#include <wplatform/window.hpp>

#include <wplatform/events.hpp>
#include <wplatform/platform.hpp>

#include <wcore/log.hpp>

#include <GLFW/glfw3.h>

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

		events::Dispatcher* dispatcher_of(GLFWwindow* window) noexcept
		{
			return static_cast<events::Dispatcher*>(glfwGetWindowUserPointer(window));
		}

		void dispatch(GLFWwindow* window, events::Event event)
		{
			if (auto* dispatcher = dispatcher_of(window))
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

			for (int i = 0; i < count; ++i)
			{
				if (paths != nullptr && paths[i] != nullptr)
					values.emplace_back(paths[i]);
			}

			dispatch(window, events::Event{
				events::Type::drop,
				events::FileDropped{ std::move(values) } });
		}

		void install_callbacks(GLFWwindow* window, events::Dispatcher* dispatcher)
		{
			glfwSetWindowUserPointer(window, dispatcher);

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
		install_callbacks(handle, dispatcher.get());

		return Window{ handle, std::move(dispatcher) };
	}

	Window::Window(GLFWwindow* handle, std::shared_ptr<events::Dispatcher> dispatcher) noexcept
		: mHandle{ handle }
		, mDispatcher{ std::move(dispatcher) }
	{
	}

	Window::Window(Window&& other) noexcept
		: mHandle{ std::exchange(other.mHandle, nullptr) }
		, mDispatcher{ std::move(other.mDispatcher) }
	{
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			mHandle = std::exchange(other.mHandle, nullptr);
			mDispatcher = std::move(other.mDispatcher);
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
} // namespace wplatform::window
