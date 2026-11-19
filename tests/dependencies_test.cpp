#include <catch2/catch_test_macros.hpp>

#include <GLFW/glfw3.h>

#include <eventpp/eventdispatcher.h>

#include <spdlog/details/os.h>
#include <spdlog/sinks/ostream_sink.h>

#include <wcore/log.hpp>
#include <wcore/types.hpp>
#include <wcore/version.hpp>

#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

namespace
{
	struct DefaultLoggerGuard
	{
		std::shared_ptr<spdlog::logger> previous{ spdlog::default_logger() };

		~DefaultLoggerGuard()
		{
			spdlog::set_default_logger(previous);
		}
	};

	static_assert(std::is_same_v<u8, std::uint8_t>);
	static_assert(std::is_same_v<u16, std::uint16_t>);
	static_assert(std::is_same_v<u32, std::uint32_t>);
	static_assert(std::is_same_v<u64, std::uint64_t>);
	static_assert(std::is_same_v<i8, std::int8_t>);
	static_assert(std::is_same_v<i16, std::int16_t>);
	static_assert(std::is_same_v<i32, std::int32_t>);
	static_assert(std::is_same_v<i64, std::int64_t>);
	static_assert(std::is_same_v<f32, float>);
	static_assert(std::is_same_v<f64, double>);
} // namespace

TEST_CASE("GLFW is available", "[dependencies]")
{
	REQUIRE(GLFW_VERSION_MAJOR >= 3);
	REQUIRE(glfwGetVersionString() != nullptr);
}

TEST_CASE("eventpp is available", "[dependencies]")
{
	eventpp::EventDispatcher<int, void(int)> dispatcher;
	int received{ 0 };

	dispatcher.appendListener(1, [&received](int value)
	{
		received = value;
	});
	dispatcher.dispatch(1, 42);

	REQUIRE(received == 42);
}

TEST_CASE("wcore is available", "[dependencies]")
{
	REQUIRE(wcore::engine_version() == wcore::Version{
		WCORE_VERSION_MAJOR, WCORE_VERSION_MINOR, WCORE_VERSION_PATCH });
	REQUIRE(std::string{ wcore::version_str() } == WCORE_VERSION_STRING);
	REQUIRE(std::string{ wcore::version_string() } == WCORE_VERSION_STRING);
}

TEST_CASE("wcore logging is available", "[dependencies]")
{
	std::ostringstream output;
	DefaultLoggerGuard guard;

	auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(output);
	auto logger = std::make_shared<spdlog::logger>("wplatform-test", sink);
	logger->set_pattern("%v");
	logger->set_level(spdlog::level::trace);
	spdlog::set_default_logger(logger);

	WCORE_INFO("wcore {} available", "logging");

	REQUIRE(output.str() == std::string{ "wcore logging available" } + SPDLOG_EOL);
}
