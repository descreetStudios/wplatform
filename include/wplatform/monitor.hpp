#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <wplatform/types.hpp>

struct GLFWmonitor;

namespace wplatform::monitor
{
	struct Rect
	{
		Position position;
		Extent size;
	};

	struct VideoMode
	{
		Extent size;
		u32 redBits;
		u32 greenBits;
		u32 blueBits;
		u32 refreshRate;

		friend bool operator==(const VideoMode& lhs, const VideoMode& rhs) noexcept
		{
			return lhs.size.width == rhs.size.width
				&& lhs.size.height == rhs.size.height
				&& lhs.redBits == rhs.redBits
				&& lhs.greenBits == rhs.greenBits
				&& lhs.blueBits == rhs.blueBits
				&& lhs.refreshRate == rhs.refreshRate;
		}
	};

	struct GammaRamp
	{
		std::vector<u16> red;
		std::vector<u16> green;
		std::vector<u16> blue;

		std::size_t size() const noexcept
		{
			return red.size();
		}

		bool valid() const noexcept
		{
			return !red.empty()
				&& red.size() == green.size()
				&& red.size() == blue.size();
		}
	};

	class Monitor
	{
	public:
		Monitor() noexcept = default;
		explicit Monitor(GLFWmonitor* handle) noexcept;

		bool valid() const noexcept;
		explicit operator bool() const noexcept;

		GLFWmonitor* native_handle() const noexcept;

		std::string name() const;
		Position position() const noexcept;
		Rect work_area() const noexcept;
		Extent physical_size() const noexcept;
		Scale content_scale() const noexcept;

		std::optional<VideoMode> current_mode() const noexcept;
		std::vector<VideoMode> modes() const;

		void set_gamma(f32 gamma) noexcept;
		GammaRamp gamma_ramp() const;
		bool set_gamma_ramp(const GammaRamp& ramp) noexcept;

		friend bool operator==(const Monitor&, const Monitor&) noexcept = default;

	private:
		GLFWmonitor* mHandle{ nullptr };
	};

	std::vector<Monitor> all();
	std::optional<Monitor> primary();
	std::optional<Monitor> at(Position position);
	std::size_t count() noexcept;
} // namespace wplatform::monitor
