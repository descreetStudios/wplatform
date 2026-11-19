#pragma once

#include <compare>
#include <iosfwd>
#include <string>
#include <string_view>

#include <wcore/types.hpp>

#include <wplatform/version.generated.hpp>

namespace wplatform
{
	struct Version
	{
		u32 major;
		u32 minor;
		u32 patch;

		constexpr auto operator<=>(const Version&) const = default;
	};

	inline constexpr Version CurrentVersion{
		WPLATFORM_VERSION_MAJOR,
		WPLATFORM_VERSION_MINOR,
		WPLATFORM_VERSION_PATCH,
	};
	inline constexpr std::string_view CurrentVersionString{ WPLATFORM_VERSION_STRING };

	Version platform_version() noexcept;
	std::string_view version_string() noexcept;

	std::string to_string(const Version& v);
	std::ostream& operator<<(std::ostream& os, const Version& v);
} // namespace wplatform
