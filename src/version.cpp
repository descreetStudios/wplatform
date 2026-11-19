#include <wplatform/version.hpp>

#include <ostream>
#include <string>

namespace wplatform
{
	Version platform_version() noexcept
	{
		return CurrentVersion;
	}

	std::string_view version_string() noexcept
	{
		return CurrentVersionString;
	}

	std::string to_string(const Version& v)
	{
		using std::to_string;
		return to_string(v.major) + '.' +
			to_string(v.minor) + '.' + to_string(v.patch);
	}

	std::ostream& operator<<(std::ostream& os, const Version& v)
	{
		return os << to_string(v);
	}
} // namespace wplatform
