#include <wplatform/cursor.hpp>

#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <wcore/log.hpp>

#include <GLFW/glfw3.h>

#include <limits>
#include <utility>

namespace wplatform::cursor
{
	namespace
	{
		static_assert(static_cast<i32>(Shape::arrow) == 0);
		static_assert(static_cast<i32>(Shape::not_allowed) == 9);
		static_assert(static_cast<i32>(Mode::normal) == 0);
		static_assert(static_cast<i32>(Mode::disabled) == 2);

		bool can_query() noexcept
		{
			return platform::initialized() && platform::is_main_thread();
		}

		int to_glfw(Shape shape) noexcept
		{
			switch (shape)
			{
			case Shape::arrow: return GLFW_ARROW_CURSOR;
			case Shape::ibeam: return GLFW_IBEAM_CURSOR;
			case Shape::crosshair: return GLFW_CROSSHAIR_CURSOR;
			case Shape::pointing_hand: return GLFW_POINTING_HAND_CURSOR;
			case Shape::resize_ew: return GLFW_RESIZE_EW_CURSOR;
			case Shape::resize_ns: return GLFW_RESIZE_NS_CURSOR;
			case Shape::resize_nwse: return GLFW_RESIZE_NWSE_CURSOR;
			case Shape::resize_nesw: return GLFW_RESIZE_NESW_CURSOR;
			case Shape::resize_all: return GLFW_RESIZE_ALL_CURSOR;
			case Shape::not_allowed: return GLFW_NOT_ALLOWED_CURSOR;
			default: return 0;
			}
		}

		int to_glfw(Mode mode) noexcept
		{
			switch (mode)
			{
			case Mode::normal: return GLFW_CURSOR_NORMAL;
			case Mode::hidden: return GLFW_CURSOR_HIDDEN;
			case Mode::disabled: return GLFW_CURSOR_DISABLED;
			default: return 0;
			}
		}

		Mode from_glfw(int mode) noexcept
		{
			switch (mode)
			{
			case GLFW_CURSOR_HIDDEN: return Mode::hidden;
			case GLFW_CURSOR_DISABLED: return Mode::disabled;
			default: return Mode::normal;
			}
		}

		bool valid_hotspot(const Image& image, Position hotspot) noexcept
		{
			return hotspot.x >= 0
				&& hotspot.y >= 0
				&& static_cast<u32>(hotspot.x) < image.size.width
				&& static_cast<u32>(hotspot.y) < image.size.height;
		}
	} // namespace

	bool Image::valid() const noexcept
	{
		constexpr u32 maxDimension{
			static_cast<u32>(std::numeric_limits<int>::max()) };

		return size.width > 0
			&& size.height > 0
			&& size.width <= maxDimension
			&& size.height <= maxDimension
			&& pixels.size() == static_cast<std::size_t>(size.width)
				* static_cast<std::size_t>(size.height) * 4;
	}

	Cursor::Cursor(GLFWcursor* handle) noexcept
		: mHandle{ handle }
	{
	}

	Cursor::Cursor(Cursor&& other) noexcept
		: mHandle{ std::exchange(other.mHandle, nullptr) }
	{
	}

	Cursor& Cursor::operator=(Cursor&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			mHandle = std::exchange(other.mHandle, nullptr);
		}

		return *this;
	}

	Cursor::~Cursor()
	{
		destroy();
	}

	void Cursor::destroy() noexcept
	{
		if (mHandle == nullptr)
			return;

		if (!platform::initialized())
		{
			mHandle = nullptr;
			return;
		}

		if (!platform::is_main_thread())
		{
			WCORE_ERROR("wplatform: a cursor must be destroyed on the main thread");
			mHandle = nullptr;
			return;
		}

		glfwDestroyCursor(mHandle);
		mHandle = nullptr;
	}

	std::optional<Cursor> Cursor::create(Shape shape)
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor creation requires an initialized platform on the main thread");
			return std::nullopt;
		}

		const int value{ to_glfw(shape) };
		if (value == 0)
		{
			WCORE_ERROR("wplatform: invalid cursor shape {}", static_cast<i32>(shape));
			return std::nullopt;
		}

		GLFWcursor* handle{ glfwCreateStandardCursor(value) };
		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: failed to create standard cursor {}", static_cast<i32>(shape));
			return std::nullopt;
		}

		return std::optional<Cursor>{ Cursor{ handle } };
	}

	std::optional<Cursor> Cursor::create(const Image& image, Position hotspot)
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor creation requires an initialized platform on the main thread");
			return std::nullopt;
		}

		if (!image.valid())
		{
			WCORE_ERROR("wplatform: cursor image must be a non-empty RGBA buffer matching its dimensions");
			return std::nullopt;
		}

		if (!valid_hotspot(image, hotspot))
		{
			WCORE_ERROR("wplatform: cursor hotspot ({}, {}) is outside the image",
				hotspot.x, hotspot.y);
			return std::nullopt;
		}

		GLFWimage native{};
		native.width = static_cast<int>(image.size.width);
		native.height = static_cast<int>(image.size.height);
		native.pixels = const_cast<unsigned char*>(image.pixels.data());

		GLFWcursor* handle{ glfwCreateCursor(&native, hotspot.x, hotspot.y) };
		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: failed to create cursor image {}x{}",
				image.size.width, image.size.height);
			return std::nullopt;
		}

		return std::optional<Cursor>{ Cursor{ handle } };
	}

	bool Cursor::valid() const noexcept
	{
		return mHandle != nullptr;
	}

	Cursor::operator bool() const noexcept
	{
		return mHandle != nullptr;
	}

	GLFWcursor* Cursor::native_handle() const noexcept
	{
		return mHandle;
	}

	bool set(window::Window& window, const Cursor& cursor) noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor changes require an initialized platform on the main thread");
			return false;
		}

		GLFWwindow* handle{ window.native_handle() };
		if (handle == nullptr || !cursor.valid())
		{
			WCORE_ERROR("wplatform: setting a cursor requires a valid window and cursor");
			return false;
		}

		glfwSetCursor(handle, cursor.native_handle());
		return true;
	}

	void reset(window::Window& window) noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor changes require an initialized platform on the main thread");
			return;
		}

		GLFWwindow* handle{ window.native_handle() };
		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: resetting a cursor requires a valid window");
			return;
		}

		glfwSetCursor(handle, nullptr);
	}

	bool set_mode(window::Window& window, Mode mode) noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor changes require an initialized platform on the main thread");
			return false;
		}

		GLFWwindow* handle{ window.native_handle() };
		const int value{ to_glfw(mode) };

		if (handle == nullptr || value == 0)
		{
			WCORE_ERROR("wplatform: setting a cursor mode requires a valid window and mode");
			return false;
		}

		glfwSetInputMode(handle, GLFW_CURSOR, value);
		return true;
	}

	Mode mode(const window::Window& window) noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor queries require an initialized platform on the main thread");
			return Mode::normal;
		}

		GLFWwindow* handle{ window.native_handle() };
		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: querying a cursor mode requires a valid window");
			return Mode::normal;
		}

		return from_glfw(glfwGetInputMode(handle, GLFW_CURSOR));
	}

	bool raw_motion_supported() noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor queries require an initialized platform on the main thread");
			return false;
		}

		return glfwRawMouseMotionSupported() == GLFW_TRUE;
	}

	bool set_raw_motion(window::Window& window, bool enabled) noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor changes require an initialized platform on the main thread");
			return false;
		}

		GLFWwindow* handle{ window.native_handle() };
		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: setting raw mouse motion requires a valid window");
			return false;
		}

		if (enabled && !raw_motion_supported())
		{
			WCORE_ERROR("wplatform: raw mouse motion is not supported by this platform");
			return false;
		}

		glfwSetInputMode(handle, GLFW_RAW_MOUSE_MOTION, enabled ? GLFW_TRUE : GLFW_FALSE);
		return true;
	}

	bool raw_motion(const window::Window& window) noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: cursor queries require an initialized platform on the main thread");
			return false;
		}

		GLFWwindow* handle{ window.native_handle() };
		if (handle == nullptr)
		{
			WCORE_ERROR("wplatform: querying raw mouse motion requires a valid window");
			return false;
		}

		return glfwGetInputMode(handle, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE;
	}
} // namespace wplatform::cursor
