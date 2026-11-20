#include <wplatform/events.hpp>
#include <wplatform/platform.hpp>
#include <wplatform/window.hpp>

#include <catch2/catch_test_macros.hpp>

#include <wcore/log.hpp>

#include <algorithm>
#include <utility>
#include <vector>

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

	wplatform::window::Config make_config()
	{
		wplatform::window::Config config;
		config.title = "wplatform events test";
		config.size = { 640, 480 };
		return config;
	}

	bool contains(const std::vector<wplatform::events::Type>& types,
		wplatform::events::Type type)
	{
		return std::find(types.begin(), types.end(), type) != types.end();
	}
} // namespace

TEST_CASE("dispatcher delivers events to matching listeners", "[events]")
{
	wplatform::events::Dispatcher dispatcher;

	int resizes{ 0 };
	int keys{ 0 };
	wplatform::Extent resized{};

	dispatcher.add_listener(wplatform::events::Type::resize,
		[&](const wplatform::events::Event& event)
		{
			++resizes;
			if (const auto* data = event.get<wplatform::events::Resized>())
				resized = data->size;
		});

	dispatcher.add_listener(wplatform::events::Type::key,
		[&](const wplatform::events::Event&)
		{
			++keys;
		});

	dispatcher.dispatch(wplatform::events::Event{
		wplatform::events::Type::resize,
		wplatform::events::Resized{ wplatform::Extent{ 640, 480 } } });

	REQUIRE(resizes == 1);
	REQUIRE(keys == 0);
	REQUIRE(resized.width == 640);
	REQUIRE(resized.height == 480);
}

TEST_CASE("wildcard listeners receive every event", "[events]")
{
	wplatform::events::Dispatcher dispatcher;

	std::vector<wplatform::events::Type> seen;
	dispatcher.add_listener([&](const wplatform::events::Event& event)
	{
		seen.push_back(event.type);
	});

	dispatcher.dispatch(wplatform::events::Event{
		wplatform::events::Type::key,
		wplatform::events::KeyEvent{
			wplatform::events::Key::a,
			30,
			wplatform::events::Action::press,
			{} } });

	dispatcher.dispatch(wplatform::events::Event{
		wplatform::events::Type::refresh });

	REQUIRE(seen.size() == 2);
	REQUIRE(seen[0] == wplatform::events::Type::key);
	REQUIRE(seen[1] == wplatform::events::Type::refresh);
}

TEST_CASE("event payloads expose typed data", "[events]")
{
	wplatform::events::Event event{
		wplatform::events::Type::key,
		wplatform::events::KeyEvent{
			wplatform::events::Key::a, 30,
			wplatform::events::Action::press,
			wplatform::events::Modifiers{ true, false, false, false, false, false } } };

	REQUIRE(event.is(wplatform::events::Type::key));
	REQUIRE_FALSE(event.is(wplatform::events::Type::resize));

	const auto* key = event.get<wplatform::events::KeyEvent>();
	REQUIRE(key != nullptr);
	REQUIRE(key->key == wplatform::events::Key::a);
	REQUIRE(key->scancode == 30);
	REQUIRE(key->action == wplatform::events::Action::press);
	REQUIRE(key->modifiers.shift);
	REQUIRE_FALSE(key->modifiers.control);

	REQUIRE(event.get<wplatform::events::Resized>() == nullptr);
}

TEST_CASE("listeners can be removed", "[events]")
{
	wplatform::events::Dispatcher dispatcher;

	int calls{ 0 };
	const auto id = dispatcher.add_listener(wplatform::events::Type::focus,
		[&](const wplatform::events::Event&)
		{
			++calls;
		});

	REQUIRE(id != wplatform::events::Dispatcher::InvalidListener);
	REQUIRE(dispatcher.listener_count() == 1);

	dispatcher.dispatch(wplatform::events::Event{
		wplatform::events::Type::focus,
		wplatform::events::FocusChanged{ true } });
	REQUIRE(calls == 1);

	REQUIRE(dispatcher.remove_listener(id));
	REQUIRE_FALSE(dispatcher.remove_listener(id));
	REQUIRE_FALSE(dispatcher.remove_listener(
		wplatform::events::Dispatcher::InvalidListener));
	REQUIRE(dispatcher.listener_count() == 0);

	dispatcher.dispatch(wplatform::events::Event{
		wplatform::events::Type::focus,
		wplatform::events::FocusChanged{ false } });
	REQUIRE(calls == 1);
}

TEST_CASE("clear removes every listener", "[events]")
{
	wplatform::events::Dispatcher dispatcher;

	int calls{ 0 };
	dispatcher.add_listener(wplatform::events::Type::key,
		[&](const wplatform::events::Event&) { ++calls; });
	dispatcher.add_listener([&](const wplatform::events::Event&) { ++calls; });

	REQUIRE(dispatcher.listener_count() == 2);

	dispatcher.clear();
	REQUIRE(dispatcher.listener_count() == 0);

	dispatcher.dispatch(wplatform::events::Event{ wplatform::events::Type::key });
	REQUIRE(calls == 0);
}

TEST_CASE("listener presence can be queried", "[events]")
{
	wplatform::events::Dispatcher dispatcher;

	REQUIRE_FALSE(dispatcher.has_listeners(wplatform::events::Type::key));

	dispatcher.add_listener(wplatform::events::Type::key,
		[](const wplatform::events::Event&) {});
	REQUIRE(dispatcher.has_listeners(wplatform::events::Type::key));
	REQUIRE_FALSE(dispatcher.has_listeners(wplatform::events::Type::resize));

	dispatcher.add_listener([](const wplatform::events::Event&) {});
	REQUIRE(dispatcher.has_listeners(wplatform::events::Type::resize));
}

TEST_CASE("dispatchers are movable", "[events]")
{
	wplatform::events::Dispatcher first;

	int calls{ 0 };
	first.add_listener(wplatform::events::Type::close,
		[&](const wplatform::events::Event&) { ++calls; });

	wplatform::events::Dispatcher second{ std::move(first) };
	second.dispatch(wplatform::events::Event{ wplatform::events::Type::close });

	REQUIRE(calls == 1);
}

TEST_CASE("event processing requires an initialized platform", "[events]")
{
	wcore::log::LogSilencer silence;
	wplatform::platform::shutdown();

	REQUIRE_FALSE(wplatform::events::poll());
	REQUIRE_FALSE(wplatform::events::wait());
	REQUIRE_FALSE(wplatform::events::wait(0.0));
}

TEST_CASE("windows own an event dispatcher", "[events][window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());
	REQUIRE(window->dispatcher() != nullptr);
	REQUIRE(wplatform::events::poll());
}

TEST_CASE("window size changes dispatch resize events", "[events][window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());

	int resizes{ 0 };
	wplatform::Extent resized{};
	window->dispatcher()->add_listener(wplatform::events::Type::resize,
		[&](const wplatform::events::Event& event)
		{
			++resizes;
			if (const auto* data = event.get<wplatform::events::Resized>())
				resized = data->size;
		});

	window->set_size(800, 600);

	REQUIRE(resizes == 1);
	REQUIRE(resized.width == 800);
	REQUIRE(resized.height == 600);
}

TEST_CASE("window state changes dispatch state events", "[events][window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());

	std::vector<wplatform::events::Type> types;
	window->dispatcher()->add_listener([&](const wplatform::events::Event& event)
	{
		types.push_back(event.type);
	});

	window->iconify();
	window->restore();
	window->maximize();
	window->restore();

	REQUIRE(contains(types, wplatform::events::Type::iconify));
	REQUIRE(contains(types, wplatform::events::Type::maximize));
}

TEST_CASE("window events survive moves", "[events][window]")
{
	PlatformGuard guard;

	auto window = wplatform::window::Window::create(make_config());
	REQUIRE(window.has_value());

	auto* dispatcher = window->dispatcher();
	int resizes{ 0 };
	dispatcher->add_listener(wplatform::events::Type::resize,
		[&](const wplatform::events::Event&) { ++resizes; });

	wplatform::window::Window moved{ std::move(*window) };
	REQUIRE(moved.dispatcher() == dispatcher);
	REQUIRE(window->dispatcher() == nullptr);

	moved.set_size(1024, 768);

	REQUIRE(resizes == 1);
}
