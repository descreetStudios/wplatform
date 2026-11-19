#pragma once

#include <functional>
#include <string>
#include <string_view>

#include <wplatform/version.hpp>

namespace wplatform::platform
{
	enum class Backend
	{
		any,
		win32,
		cocoa,
		x11,
		wayland,
		null,
	};

	struct Error
	{
		int code;
		std::string message;
	};

	struct Config
	{
		Backend backend{ Backend::any };
		bool joystickHatsAsButtons{ true };
	};

	using ErrorHandler = std::function<void(const Error&)>;

	bool init(const Config& config = {});
	void shutdown() noexcept;

	bool initialized() noexcept;

	Backend backend() noexcept;
	std::string_view backend_name() noexcept;
	std::string_view backend_name(Backend backend) noexcept;
	bool backend_supported(Backend backend) noexcept;

	Version glfw_version() noexcept;
	Version glfw_compiled_version() noexcept;
	bool glfw_versions_match() noexcept;

	bool is_main_thread() noexcept;

	void set_error_handler(ErrorHandler handler);
} // namespace wplatform::platform
