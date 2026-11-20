#include <wplatform/monitor.hpp>
#include <wplatform/platform.hpp>

#include <wcore/log.hpp>

#include "internal.hpp"

#include <GLFW/glfw3.h>

namespace wplatform::monitor
{
	namespace
	{
		bool can_query() noexcept
		{
			return platform::initialized() && platform::is_main_thread();
		}

		bool connected(GLFWmonitor* handle) noexcept
		{
			if (handle == nullptr)
				return false;

			int count{ 0 };
			GLFWmonitor** monitors{ glfwGetMonitors(&count) };
			if (monitors == nullptr)
				return false;

			for (int i{ 0 }; i < count; ++i)
			{
				if (monitors[i] == handle)
					return true;
			}

			return false;
		}

		VideoMode to_video_mode(const GLFWvidmode& mode) noexcept
		{
			return VideoMode{
				Extent{
					static_cast<u32>(mode.width),
					static_cast<u32>(mode.height) },
				static_cast<u32>(mode.redBits),
				static_cast<u32>(mode.greenBits),
				static_cast<u32>(mode.blueBits),
				static_cast<u32>(mode.refreshRate) };
		}

		void on_monitor(GLFWmonitor* handle, int event)
		{
			if (handle == nullptr)
				return;

			if (event == GLFW_CONNECTED)
			{
				events::detail::broadcast(events::Event{
					events::Type::monitor_connected,
					events::MonitorConnected{ Monitor{ handle } } });
				return;
			}

			if (event == GLFW_DISCONNECTED)
			{
				const char* name{ glfwGetMonitorName(handle) };
				events::detail::broadcast(events::Event{
					events::Type::monitor_disconnected,
					events::MonitorDisconnected{
						Monitor{ handle },
						name != nullptr ? std::string{ name } : std::string{} } });
			}
		}
	} // namespace

	Monitor::Monitor(GLFWmonitor* handle) noexcept
		: mHandle{ handle }
	{
	}

	bool Monitor::valid() const noexcept
	{
		return can_query() && connected(mHandle);
	}

	Monitor::operator bool() const noexcept
	{
		return valid();
	}

	GLFWmonitor* Monitor::native_handle() const noexcept
	{
		return mHandle;
	}

	std::string Monitor::name() const
	{
		if (!valid())
			return {};

		const char* value{ glfwGetMonitorName(mHandle) };
		return value != nullptr ? std::string{ value } : std::string{};
	}

	Position Monitor::position() const noexcept
	{
		if (!valid())
			return {};

		int x{ 0 };
		int y{ 0 };
		glfwGetMonitorPos(mHandle, &x, &y);

		return Position{
			static_cast<i32>(x),
			static_cast<i32>(y),
		};
	}

	Rect Monitor::work_area() const noexcept
	{
		if (!valid())
			return {};

		int x{ 0 };
		int y{ 0 };
		int width{ 0 };
		int height{ 0 };
		glfwGetMonitorWorkarea(mHandle, &x, &y, &width, &height);

		return Rect{
			Position{ static_cast<i32>(x), static_cast<i32>(y) },
			Extent{ static_cast<u32>(width), static_cast<u32>(height) },
		};
	}

	Extent Monitor::physical_size() const noexcept
	{
		if (!valid())
			return {};

		int width{ 0 };
		int height{ 0 };
		glfwGetMonitorPhysicalSize(mHandle, &width, &height);

		return Extent{
			static_cast<u32>(width),
			static_cast<u32>(height),
		};
	}

	Scale Monitor::content_scale() const noexcept
	{
		if (!valid())
			return { 1.0f, 1.0f };

		float x{ 1.0f };
		float y{ 1.0f };
		glfwGetMonitorContentScale(mHandle, &x, &y);

		return Scale{ x, y };
	}

	std::optional<VideoMode> Monitor::current_mode() const noexcept
	{
		if (!valid())
			return std::nullopt;

		const GLFWvidmode* mode{ glfwGetVideoMode(mHandle) };
		if (mode == nullptr)
			return std::nullopt;

		return to_video_mode(*mode);
	}

	std::vector<VideoMode> Monitor::modes() const
	{
		if (!valid())
			return {};

		int count{ 0 };
		const GLFWvidmode* modes{ glfwGetVideoModes(mHandle, &count) };
		if (modes == nullptr || count <= 0)
			return {};

		std::vector<VideoMode> result;
		result.reserve(static_cast<std::size_t>(count));

		for (int i{ 0 }; i < count; ++i)
			result.push_back(to_video_mode(modes[i]));

		return result;
	}

	void Monitor::set_gamma(f32 gamma) noexcept
	{
		if (!valid())
			return;

		if (gamma <= 0.0f)
		{
			WCORE_ERROR("wplatform: monitor gamma must be positive ({})", gamma);
			return;
		}

		glfwSetGamma(mHandle, gamma);
	}

	GammaRamp Monitor::gamma_ramp() const
	{
		if (!valid())
			return {};

		const GLFWgammaramp* ramp{ glfwGetGammaRamp(mHandle) };
		if (ramp == nullptr || ramp->size == 0)
			return {};

		GammaRamp result;
		result.red.assign(ramp->red, ramp->red + ramp->size);
		result.green.assign(ramp->green, ramp->green + ramp->size);
		result.blue.assign(ramp->blue, ramp->blue + ramp->size);

		return result;
	}

	bool Monitor::set_gamma_ramp(const GammaRamp& ramp) noexcept
	{
		if (!valid() || !ramp.valid())
			return false;

		std::vector<u16> red{ ramp.red };
		std::vector<u16> green{ ramp.green };
		std::vector<u16> blue{ ramp.blue };

		GLFWgammaramp value{};
		value.red = red.data();
		value.green = green.data();
		value.blue = blue.data();
		value.size = static_cast<unsigned int>(red.size());

		glfwSetGammaRamp(mHandle, &value);
		return true;
	}

	std::vector<Monitor> all()
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: monitor queries require an initialized platform on the main thread");
			return {};
		}

		int count{ 0 };
		GLFWmonitor** handles{ glfwGetMonitors(&count) };
		if (handles == nullptr || count <= 0)
			return {};

		std::vector<Monitor> result;
		result.reserve(static_cast<std::size_t>(count));

		for (int i{ 0 }; i < count; ++i)
			result.emplace_back(handles[i]);

		return result;
	}

	std::optional<Monitor> primary()
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: monitor queries require an initialized platform on the main thread");
			return std::nullopt;
		}

		GLFWmonitor* handle{ glfwGetPrimaryMonitor() };
		if (handle == nullptr)
			return std::nullopt;

		return Monitor{ handle };
	}

	std::optional<Monitor> at(Position position)
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: monitor queries require an initialized platform on the main thread");
			return std::nullopt;
		}

		for (const Monitor& candidate : all())
		{
			const Position origin{ candidate.position() };
			const std::optional<VideoMode> mode{ candidate.current_mode() };
			if (!mode)
				continue;

			const i64 x{ static_cast<i64>(position.x) };
			const i64 y{ static_cast<i64>(position.y) };
			const i64 left{ static_cast<i64>(origin.x) };
			const i64 top{ static_cast<i64>(origin.y) };
			const i64 right{ left + static_cast<i64>(mode->size.width) };
			const i64 bottom{ top + static_cast<i64>(mode->size.height) };

			if (x >= left && x < right && y >= top && y < bottom)
				return candidate;
		}

		return std::nullopt;
	}

	std::size_t count() noexcept
	{
		if (!can_query())
		{
			WCORE_ERROR("wplatform: monitor queries require an initialized platform on the main thread");
			return 0;
		}

		int value{ 0 };
		glfwGetMonitors(&value);

		return value > 0 ? static_cast<std::size_t>(value) : std::size_t{ 0 };
	}

	namespace detail
	{
		void install_callback() noexcept
		{
			glfwSetMonitorCallback(on_monitor);
		}
	} // namespace detail
} // namespace wplatform::monitor
