#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace wplatform::window
{
	class Window;
}

namespace wplatform::clipboard
{
	std::optional<std::string> get(const window::Window& window);

	bool set(const window::Window& window, std::string_view text) noexcept;
} // namespace wplatform::clipboard
