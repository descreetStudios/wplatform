#include <wplatform/window.hpp>

#include <wplatform/platform.hpp>

#include <wcore/log.hpp>

#include <GLFW/glfw3.h>

#include <string>
#include <utility>

namespace wplatform::window
{
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

		return Window{ handle };
	}

	Window::Window(GLFWwindow* handle) noexcept
		: mHandle{ handle }
	{
	}

	Window::Window(Window&& other) noexcept
		: mHandle{ std::exchange(other.mHandle, nullptr) }
	{
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			mHandle = std::exchange(other.mHandle, nullptr);
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
