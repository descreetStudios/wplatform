#include <wplatform/clipboard.hpp>

#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <wcore/log.hpp>

#include <GLFW/glfw3.h>

#include <string>

namespace wplatform::clipboard
{
	namespace
	{
		bool can_query() noexcept
		{
			return platform::initialized() && platform::is_main_thread();
		}
	} // namespace

	std::optional<std::string> get(const window::Window& window)
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: clipboard queries require an initialized platform on the main thread");
			return std::nullopt;
		}

		GLFWwindow* handle{ window.native_handle() };
		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: querying the clipboard requires a valid window");
			return std::nullopt;
		}

		const char* value{ glfwGetClipboardString(handle) };
		if (value == nullptr)
			return std::nullopt;

		return std::string{ value };
	}

	bool set(const window::Window& window, std::string_view text) noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: clipboard changes require an initialized platform on the main thread");
			return false;
		}

		GLFWwindow* handle{ window.native_handle() };
		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: setting the clipboard requires a valid window");
			return false;
		}

		const std::string value{ text };
		glfwSetClipboardString(handle, value.c_str());
		return true;
	}
} // namespace wplatform::clipboard
