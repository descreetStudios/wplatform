#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <wcore/types.hpp>

struct GLFWwindow;

namespace wplatform::window
{
	struct Extent
	{
		u32 width;
		u32 height;
	};

	struct Position
	{
		i32 x;
		i32 y;
	};

	struct Scale
	{
		f32 x;
		f32 y;
	};

	struct Config
	{
		std::string title{ "wplatform" };
		Extent size{ 1280, 720 };
		bool resizable{ true };
		bool visible{ true };
		bool decorated{ true };
		bool focused{ true };
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

		bool should_close() const noexcept;
		void request_close() noexcept;

		Extent size() const noexcept;
		Extent framebuffer_size() const noexcept;
		void set_size(u32 width, u32 height) noexcept;

		Position position() const noexcept;
		void set_position(i32 x, i32 y) noexcept;

		Scale content_scale() const noexcept;

		std::string_view title() const noexcept;
		void set_title(std::string_view title);

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

	private:
		explicit Window(GLFWwindow* handle) noexcept;

		void destroy() noexcept;

		GLFWwindow* mHandle{ nullptr };
	};
} // namespace wplatform::window
