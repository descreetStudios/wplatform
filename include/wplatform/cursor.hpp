#pragma once

#include <optional>
#include <vector>

#include <wplatform/types.hpp>

struct GLFWcursor;

namespace wplatform::window
{
	class Window;
}

namespace wplatform::cursor
{
	enum class Shape : i32
	{
		arrow = 0,
		ibeam,
		crosshair,
		pointing_hand,
		resize_ew,
		resize_ns,
		resize_nwse,
		resize_nesw,
		resize_all,
		not_allowed,
	};

	enum class Mode : i32
	{
		normal = 0,
		hidden,
		disabled,
	};

	struct Image
	{
		Extent size{};
		std::vector<u8> pixels;

		bool valid() const noexcept;
	};

	class Cursor
	{
	public:
		static std::optional<Cursor> create(Shape shape);
		static std::optional<Cursor> create(const Image& image, Position hotspot = {});

		Cursor() noexcept = default;
		Cursor(Cursor&& other) noexcept;
		Cursor& operator=(Cursor&& other) noexcept;
		~Cursor();

		Cursor(const Cursor&) = delete;
		Cursor& operator=(const Cursor&) = delete;

		bool valid() const noexcept;
		explicit operator bool() const noexcept;

		GLFWcursor* native_handle() const noexcept;

	private:
		explicit Cursor(GLFWcursor* handle) noexcept;

		void destroy() noexcept;

		GLFWcursor* mHandle{ nullptr };
	};

	bool set(window::Window& window, const Cursor& cursor) noexcept;
	void reset(window::Window& window) noexcept;

	bool set_mode(window::Window& window, Mode mode) noexcept;
	Mode mode(const window::Window& window) noexcept;

	bool raw_motion_supported() noexcept;

	bool set_raw_motion(window::Window& window, bool enabled) noexcept;
	bool raw_motion(const window::Window& window) noexcept;
} // namespace wplatform::cursor
