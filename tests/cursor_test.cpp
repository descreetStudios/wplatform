#include <wplatform/cursor.hpp>
#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <catch2/catch_test_macros.hpp>

#include <wcore/log.hpp>

#include <utility>

namespace
{
	struct PlatformGuard
	{
		PlatformGuard()
		{
			wplatform::platform::Config config;
			config.backend = wplatform::platform::Backend::null;
			REQUIRE(wplatform::platform::init(config));
		}

		~PlatformGuard()
		{
			wplatform::platform::shutdown();
		}
	};

	wplatform::cursor::Image make_image()
	{
		wplatform::cursor::Image image;
		image.size = { 2, 2 };
		image.pixels.assign(2 * 2 * 4, 0);
		return image;
	}
} // namespace

TEST_CASE("cursor creation requires an initialized platform", "[cursor]")
{
	wcore::log::LogSilencer silence;
	wplatform::platform::shutdown();

	REQUIRE_FALSE(wplatform::cursor::Cursor::create(wplatform::cursor::Shape::arrow).has_value());
	REQUIRE_FALSE(wplatform::cursor::Cursor::create(make_image()).has_value());
	REQUIRE_FALSE(wplatform::cursor::raw_motion_supported());

	const wplatform::cursor::Cursor cursor;
	REQUIRE_FALSE(cursor.valid());
	REQUIRE_FALSE(static_cast<bool>(cursor));
	REQUIRE(cursor.native_handle() == nullptr);
}

TEST_CASE("standard cursors can be created on the null backend", "[cursor]")
{
	wcore::log::LogSilencer silence;
	PlatformGuard guard;

	for (i32 i{ 0 }; i <= static_cast<i32>(wplatform::cursor::Shape::not_allowed); ++i)
	{
		const auto shape{ static_cast<wplatform::cursor::Shape>(i) };
		const auto cursor{ wplatform::cursor::Cursor::create(shape) };

		REQUIRE(cursor.has_value());
		REQUIRE(cursor->valid());
		REQUIRE(cursor->native_handle() != nullptr);
	}

	REQUIRE_FALSE(wplatform::cursor::Cursor::create(
		static_cast<wplatform::cursor::Shape>(99)).has_value());
}

TEST_CASE("cursors are move-only and release their handle", "[cursor]")
{
	PlatformGuard guard;

	auto first{ wplatform::cursor::Cursor::create(wplatform::cursor::Shape::arrow) };
	REQUIRE(first.has_value());

	auto* handle{ first->native_handle() };

	wplatform::cursor::Cursor moved{ std::move(*first) };
	REQUIRE(moved.valid());
	REQUIRE(moved.native_handle() == handle);
	REQUIRE_FALSE(first->valid());
	REQUIRE(first->native_handle() == nullptr);

	auto other{ wplatform::cursor::Cursor::create(wplatform::cursor::Shape::crosshair) };
	REQUIRE(other.has_value());

	*other = std::move(moved);
	REQUIRE(other->valid());
	REQUIRE(other->native_handle() == handle);
	REQUIRE_FALSE(moved.valid());
}

TEST_CASE("image cursors validate their pixels and hotspot", "[cursor]")
{
	wcore::log::LogSilencer silence;
	PlatformGuard guard;

	const auto image{ make_image() };
	REQUIRE(image.valid());

	const auto cursor{ wplatform::cursor::Cursor::create(image, { 1, 1 }) };
	REQUIRE(cursor.has_value());
	REQUIRE(cursor->valid());

	REQUIRE_FALSE(wplatform::cursor::Cursor::create(wplatform::cursor::Image{}).has_value());

	auto empty{ image };
	empty.pixels.clear();
	REQUIRE_FALSE(empty.valid());
	REQUIRE_FALSE(wplatform::cursor::Cursor::create(empty).has_value());

	auto mismatched{ image };
	mismatched.pixels.pop_back();
	REQUIRE_FALSE(mismatched.valid());
	REQUIRE_FALSE(wplatform::cursor::Cursor::create(mismatched).has_value());

	REQUIRE_FALSE(wplatform::cursor::Cursor::create(image, { -1, 0 }).has_value());
	REQUIRE_FALSE(wplatform::cursor::Cursor::create(image, { 0, 2 }).has_value());
	REQUIRE_FALSE(wplatform::cursor::Cursor::create(image, { 2, 0 }).has_value());
}

TEST_CASE("cursor modes can be changed", "[cursor]")
{
	wcore::log::LogSilencer silence;
	PlatformGuard guard;

	wplatform::window::Config config;
	config.title = "wplatform cursor mode test";
	config.size = { 320, 240 };

	auto window{ wplatform::window::Window::create(config) };
	REQUIRE(window.has_value());

	REQUIRE(wplatform::cursor::mode(*window) == wplatform::cursor::Mode::normal);

	REQUIRE(wplatform::cursor::set_mode(*window, wplatform::cursor::Mode::hidden));
	REQUIRE(wplatform::cursor::mode(*window) == wplatform::cursor::Mode::hidden);

	REQUIRE(wplatform::cursor::set_mode(*window, wplatform::cursor::Mode::disabled));
	REQUIRE(wplatform::cursor::mode(*window) == wplatform::cursor::Mode::disabled);

	REQUIRE(wplatform::cursor::set_mode(*window, wplatform::cursor::Mode::normal));
	REQUIRE(wplatform::cursor::mode(*window) == wplatform::cursor::Mode::normal);

	REQUIRE_FALSE(wplatform::cursor::set_mode(
		*window, static_cast<wplatform::cursor::Mode>(99)));
	REQUIRE(wplatform::cursor::mode(*window) == wplatform::cursor::Mode::normal);
}

TEST_CASE("cursors can be attached to and reset from a window", "[cursor]")
{
	wcore::log::LogSilencer silence;
	PlatformGuard guard;

	wplatform::window::Config config;
	config.title = "wplatform cursor set test";
	config.size = { 320, 240 };

	auto window{ wplatform::window::Window::create(config) };
	REQUIRE(window.has_value());

	auto cursor{ wplatform::cursor::Cursor::create(wplatform::cursor::Shape::pointing_hand) };
	REQUIRE(cursor.has_value());

	REQUIRE(wplatform::cursor::set(*window, *cursor));
	wplatform::cursor::reset(*window);

	const wplatform::cursor::Cursor invalid;
	REQUIRE_FALSE(wplatform::cursor::set(*window, invalid));

	auto movedFrom{ wplatform::window::Window::create(config) };
	REQUIRE(movedFrom.has_value());

	wplatform::window::Window destination{ std::move(*movedFrom) };
	REQUIRE(destination.valid());
	REQUIRE(movedFrom->native_handle() == nullptr);

	REQUIRE_FALSE(wplatform::cursor::set(*movedFrom, *cursor));
	wplatform::cursor::reset(*movedFrom);
	REQUIRE_FALSE(wplatform::cursor::set_mode(
		*movedFrom, wplatform::cursor::Mode::hidden));
	REQUIRE(wplatform::cursor::mode(*movedFrom) == wplatform::cursor::Mode::normal);
}

TEST_CASE("raw mouse motion can be toggled", "[cursor]")
{
	PlatformGuard guard;

	wplatform::window::Config config;
	config.title = "wplatform raw mouse motion test";
	config.size = { 320, 240 };

	auto window{ wplatform::window::Window::create(config) };
	REQUIRE(window.has_value());

	REQUIRE(wplatform::cursor::raw_motion_supported());
	REQUIRE_FALSE(wplatform::cursor::raw_motion(*window));

	REQUIRE(wplatform::cursor::set_raw_motion(*window, true));
	REQUIRE(wplatform::cursor::raw_motion(*window));

	REQUIRE(wplatform::cursor::set_raw_motion(*window, false));
	REQUIRE_FALSE(wplatform::cursor::raw_motion(*window));
}
