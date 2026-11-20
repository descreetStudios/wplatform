#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <wplatform/monitor.hpp>
#include <wplatform/types.hpp>

struct GLFWwindow;

namespace wplatform::events
{
	class Dispatcher;
}

namespace wplatform::input
{
	class State;
}

namespace wplatform::window
{
	using wplatform::Extent;
	using wplatform::Position;
	using wplatform::Scale;

	struct SizeLimits
	{
		std::optional<u32> minWidth;
		std::optional<u32> minHeight;
		std::optional<u32> maxWidth;
		std::optional<u32> maxHeight;

		bool any() const noexcept
		{
			return minWidth.has_value() || minHeight.has_value()
				|| maxWidth.has_value() || maxHeight.has_value();
		}
	};

	struct Icon
	{
		Extent size{};
		std::vector<u8> pixels;

		bool valid() const noexcept
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
	};

	struct Config
	{
		std::string title{ "wplatform" };
		Extent size{ 1280, 720 };
		bool resizable{ true };
		bool visible{ true };
		bool decorated{ true };
		bool focused{ true };
		bool maximized{ false };
		bool floating{ false };
		bool autoIconify{ true };
		bool mousePassthrough{ false };
		bool transparentFramebuffer{ false };
		SizeLimits sizeLimits;
	};

	class Window
	{
	public:
		static std::optional<Window> create(const Config& config = {});

		Window(Window&& other) noexcept;
		Window& operator=(Window&& other) noexcept;
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		bool valid() const noexcept;
		GLFWwindow* native_handle() const noexcept;

		events::Dispatcher* dispatcher() noexcept;
		const events::Dispatcher* dispatcher() const noexcept;

		wplatform::input::State* input() noexcept;
		const wplatform::input::State* input() const noexcept;

		bool should_close() const noexcept;
		void request_close() noexcept;

		Extent size() const noexcept;
		Extent framebuffer_size() const noexcept;
		void set_size(u32 width, u32 height) noexcept;
		void set_size_limits(const SizeLimits& limits) noexcept;
		void clear_size_limits() noexcept;
		void set_aspect_ratio(u32 numerator, u32 denominator) noexcept;
		void clear_aspect_ratio() noexcept;

		Position position() const noexcept;
		void set_position(i32 x, i32 y) noexcept;

		Scale content_scale() const noexcept;

		std::string_view title() const noexcept;
		void set_title(std::string_view title);

		bool set_icons(std::span<const Icon> icons) noexcept;
		bool set_icon(const Icon& icon) noexcept;

		f32 opacity() const noexcept;
		void set_opacity(f32 opacity) noexcept;
		void request_attention() noexcept;

		void show() noexcept;
		void hide() noexcept;
		void focus() noexcept;
		void iconify() noexcept;
		void restore() noexcept;
		void maximize() noexcept;

		bool visible() const noexcept;
		bool focused() const noexcept;
		bool iconified() const noexcept;
		bool maximized() const noexcept;
		bool resizable() const noexcept;
		void set_resizable(bool resizable) noexcept;
		bool hovered() const noexcept;
		bool transparent_framebuffer() const noexcept;

		bool floating() const noexcept;
		void set_floating(bool enabled) noexcept;
		bool auto_iconify() const noexcept;
		void set_auto_iconify(bool enabled) noexcept;
		bool mouse_passthrough() const noexcept;
		void set_mouse_passthrough(bool enabled) noexcept;

		std::optional<monitor::Monitor> fullscreen_monitor() const;
		bool set_fullscreen(const monitor::Monitor& monitor,
			const monitor::VideoMode& mode) noexcept;
		bool set_windowed(Position position, Extent size) noexcept;

	private:
		explicit Window(GLFWwindow* handle,
			std::shared_ptr<events::Dispatcher> dispatcher,
			std::unique_ptr<wplatform::input::State> input) noexcept;

		void destroy() noexcept;

		GLFWwindow* mHandle{ nullptr };
		std::shared_ptr<events::Dispatcher> mDispatcher;
		std::unique_ptr<wplatform::input::State> mInput;
	};
} // namespace wplatform::window
