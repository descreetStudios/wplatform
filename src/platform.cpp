#include <wplatform/platform.hpp>

#include <wcore/log.hpp>

#include <GLFW/glfw3.h>

#include <thread>
#include <utility>

namespace wplatform::platform
{
	namespace
	{
		bool gInitialized{ false };
		bool gMainThreadKnown{ false };
		std::thread::id gMainThread;
		ErrorHandler gErrorHandler;

		int to_glfw(Backend backend) noexcept
		{
			switch (backend)
			{
			case Backend::any: return GLFW_ANY_PLATFORM;
			case Backend::win32: return GLFW_PLATFORM_WIN32;
			case Backend::cocoa: return GLFW_PLATFORM_COCOA;
			case Backend::x11: return GLFW_PLATFORM_X11;
			case Backend::wayland: return GLFW_PLATFORM_WAYLAND;
			case Backend::null: return GLFW_PLATFORM_NULL;
			default: return GLFW_ANY_PLATFORM;
			}
		}

		Backend from_glfw(int backend) noexcept
		{
			switch (backend)
			{
			case GLFW_PLATFORM_WIN32: return Backend::win32;
			case GLFW_PLATFORM_COCOA: return Backend::cocoa;
			case GLFW_PLATFORM_X11: return Backend::x11;
			case GLFW_PLATFORM_WAYLAND: return Backend::wayland;
			case GLFW_PLATFORM_NULL: return Backend::null;
			default: return Backend::any;
			}
		}

		void on_glfw_error(int code, const char* description)
		{
			const Error error{ code,
				description != nullptr ? description : "" };

			if (gErrorHandler)
			{
				gErrorHandler(error);
				return;
			}

			WCORE_ERROR("wplatform: GLFW error 0x{:04x}: {}",
				static_cast<u32>(code), error.message);
		}
	} // namespace

	bool init(const Config& config)
	{
		const std::thread::id caller{ std::this_thread::get_id() };

		if (gInitialized)
		{
			if (!is_main_thread())
			{
				WCORE_ERROR("wplatform: platform::init must be called from the main thread");
				return false;
			}

			WCORE_WARN("wplatform: platform is already initialized");
			return true;
		}

		if (gMainThreadKnown && caller != gMainThread)
		{
			WCORE_ERROR("wplatform: platform::init must be called from the main thread");
			return false;
		}

		gMainThread = caller;
		gMainThreadKnown = true;

		if (!backend_supported(config.backend))
		{
			WCORE_ERROR("wplatform: backend '{}' is unavailable on this system",
				backend_name(config.backend));
			return false;
		}

		glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS,
			config.joystickHatsAsButtons ? GLFW_TRUE : GLFW_FALSE);
		glfwInitHint(GLFW_PLATFORM, to_glfw(config.backend));

		glfwSetErrorCallback(on_glfw_error);

		if (glfwInit() == GLFW_FALSE)
		{
			WCORE_ERROR("wplatform: GLFW initialization failed");
			return false;
		}

		gInitialized = true;

		WCORE_INFO("wplatform: initialized (backend '{}', GLFW {})",
			backend_name(), to_string(glfw_version()));

		return true;
	}

	void shutdown() noexcept
	{
		if (!gInitialized)
			return;

		if (!is_main_thread())
		{
			WCORE_ERROR("wplatform: platform::shutdown must be called from the main thread");
			return;
		}

		glfwTerminate();
		gInitialized = false;

		WCORE_INFO("wplatform: shut down");
	}

	bool initialized() noexcept
	{
		return gInitialized;
	}

	Backend backend() noexcept
	{
		if (!gInitialized)
			return Backend::any;

		return from_glfw(glfwGetPlatform());
	}

	std::string_view backend_name(Backend backend) noexcept
	{
		switch (backend)
		{
		case Backend::any: return "any";
		case Backend::win32: return "win32";
		case Backend::cocoa: return "cocoa";
		case Backend::x11: return "x11";
		case Backend::wayland: return "wayland";
		case Backend::null: return "null";
		default: return "unknown";
		}
	}

	std::string_view backend_name() noexcept
	{
		return backend_name(backend());
	}

	bool backend_supported(Backend backend) noexcept
	{
		const int value{ to_glfw(backend) };
		if (value == GLFW_ANY_PLATFORM)
			return true;

		return glfwPlatformSupported(value) == GLFW_TRUE;
	}

	Version glfw_version() noexcept
	{
		int major{ 0 };
		int minor{ 0 };
		int revision{ 0 };
		glfwGetVersion(&major, &minor, &revision);

		return Version{
			static_cast<u32>(major),
			static_cast<u32>(minor),
			static_cast<u32>(revision),
		};
	}

	Version glfw_compiled_version() noexcept
	{
		return Version{
			static_cast<u32>(GLFW_VERSION_MAJOR),
			static_cast<u32>(GLFW_VERSION_MINOR),
			static_cast<u32>(GLFW_VERSION_REVISION),
		};
	}

	bool glfw_versions_match() noexcept
	{
		return glfw_version() == glfw_compiled_version();
	}

	bool is_main_thread() noexcept
	{
		return gInitialized && 
			std::this_thread::get_id() == gMainThread;
	}

	void set_error_handler(ErrorHandler handler)
	{
		gErrorHandler = std::move(handler);
	}
} // namespace wplatform::platform
