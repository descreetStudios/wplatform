<div align="center">

# wplatform

**A C++20 platform foundation for modern projects.**

<a href="https://isocpp.org/"><img src="https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white" alt="C++20"></a>
<a href="https://cmake.org/"><img src="https://img.shields.io/badge/CMake-3.24%2B-064F8C?logo=cmake&logoColor=white" alt="CMake 3.24+"></a>

</div>

`wplatform` is a small platform library for C++ projects. It wraps GLFW behind a clean C++20 API: a static library, a single CMake target (`wplatform::wplatform`), and headers under `<wplatform/...>`.

Today it provides platform initialization and backend selection, a move-only RAII window type with an event dispatcher and per-window input state, monitor and joystick enumeration with hotplug events, gamepad mappings, custom cursors and cursor modes, clipboard access, and a version header generated from the CMake project version. The target also carries its dependencies (GLFW, `wcore`, and eventpp), so consumers do not need to fetch them separately.

> **Status:** early and evolving. New modules may be added, but the package name, target name, and header layout are intended to remain stable.

## Requirements

- CMake 3.24 or newer
- A compiler with C++20 support
- Ninja, for the `linux-ninja` preset
- Network access on the first configure (Catch2, `wcore`, GLFW, and eventpp are fetched by CMake)

## Get the source

```sh
git clone https://github.com/descreetStudios/wplatform.git
cd wplatform
```

## Build

The quickest path is one of the bundled presets:

```sh
# Windows (Visual Studio)
cmake --preset windows-msvc
cmake --build --preset windows-msvc-debug

# Linux (Ninja Multi-Config)
cmake --preset linux-ninja
cmake --build build/linux-ninja --config Debug
```

Or configure manually:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Build options

| Option | Default | Description |
| --- | --- | --- |
| `WPLATFORM_BUILD_TESTS` | `ON` when top-level | Build the test suite |
| `WPLATFORM_USE_SYSTEM_GLFW` | `OFF` | Use an installed GLFW instead of fetching it |
| `WPLATFORM_USE_SYSTEM_EVENTPP` | `OFF` | Use an installed eventpp instead of fetching it |

When `wplatform` is added as a subdirectory of another project, tests default to `OFF`, so the parent build is not affected.

## Run the tests

```sh
ctest --test-dir build/windows-msvc -C Debug --output-on-failure
```

The tests use [Catch2](https://github.com/catchorg/Catch2) v3, fetched automatically at configure time. Test discovery is registered with CTest, so each `TEST_CASE` runs as an individual test.

## Use it in your project

### As a subdirectory

```cmake
add_subdirectory(third_party/wplatform)

target_link_libraries(my_app PRIVATE wplatform::wplatform)
```

### With FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(wplatform
	GIT_REPOSITORY https://github.com/descreetStudios/wplatform.git
	GIT_TAG        v0.1.0
)
FetchContent_MakeAvailable(wplatform)

target_link_libraries(my_app PRIVATE wplatform::wplatform)
```

In both cases the include directories come from the target. Include public headers by their path under `<wplatform/...>`:

```cpp
#include <wplatform/clipboard.hpp>
#include <wplatform/cursor.hpp>
#include <wplatform/events.hpp>
#include <wplatform/input.hpp>
#include <wplatform/joystick.hpp>
#include <wplatform/monitor.hpp>
#include <wplatform/platform.hpp>
#include <wplatform/version.hpp>
#include <wplatform/window.hpp>
```

## Example

```cpp
#include <wplatform/platform.hpp>
#include <wplatform/version.hpp>
#include <wplatform/window.hpp>

#include <iostream>

int main()
{
	std::cout << "wplatform " << wplatform::CurrentVersion << '\n';

	if (!wplatform::platform::init())
	{
		return 1;
	}

	{
		auto window = wplatform::window::Window::create();
		if (!window)
		{
			wplatform::platform::shutdown();
			return 1;
		}

		std::cout << window->title() << ": "
			<< window->size().width << " x " << window->size().height << '\n';
	}

	wplatform::platform::shutdown();
}
```

```text
wplatform 0.1.0
wplatform: 1280 x 720
```

## Platform and windows

Initialization happens once on the main thread. `platform::Config` lets you request a specific backend (`any`, `win32`, `cocoa`, `x11`, `wayland`, or `null`) and control the joystick-hat hint. `init` returns `false` if the backend is unavailable or GLFW fails to start:

```cpp
#include <wplatform/platform.hpp>

wplatform::platform::Config config;
config.backend = wplatform::platform::Backend::x11;

if (!wplatform::platform::init(config))
{
	return 1;
}
```

GLFW errors are reported through `wcore` logging by default. Install your own `ErrorHandler` to receive them directly:

```cpp
#include <iostream>

wplatform::platform::set_error_handler([](const wplatform::platform::Error& error)
{
	std::cerr << "GLFW " << error.code << ": " << error.message << '\n';
});
```

Use `backend()`, `backend_name()`, `backend_supported()`, `glfw_version()`, and `glfw_compiled_version()` to inspect the running platform, and `is_main_thread()` to check that calls are made on the thread that initialized it. Call `platform::shutdown()` from that same thread when you are done.

`window::Window::create` also requires the main thread and an initialized platform, and returns `std::nullopt` on failure. Windows are move-only, own their GLFW handle, and are created without a graphics context (`GLFW_NO_API`), leaving rendering setup to you. `native_handle()` exposes the `GLFWwindow*` for interop, and `window::Config` covers the title, initial size, resizability, visibility, decoration, and focus:

```cpp
wplatform::window::Config config;
config.title = "Engine";
config.size = { 1920, 1080 };
config.resizable = false;

auto window = wplatform::window::Window::create(config);
```

A window can be resized, repositioned, shown or hidden, focused, iconified, restored, and maximized; `size()`, `framebuffer_size()`, `position()`, `content_scale()`, `title()`, and the state queries reflect its current status. Destroy all windows before `platform::shutdown()` — keeping them in an inner scope (as in the example above) makes the order automatic.

## Monitors

`monitor` exposes the displays attached to the system without leaking GLFW types:

```cpp
#include <wplatform/monitor.hpp>

for (const auto& display : wplatform::monitor::all())
{
	std::cout << display.name() << ": "
		<< display.work_area().size.width << " x "
		<< display.work_area().size.height << '\n';

	if (const auto mode = display.current_mode())
	{
		// mode->size, mode->refreshRate
	}
}
```

Use `primary()` for the main display, `at({ x, y })` for the display containing a point in screen coordinates, and `count()` for just the number of displays. `Monitor` is a lightweight handle: `valid()` reports whether it is still connected, `name()`, `position()`, `work_area()`, `physical_size()`, and `content_scale()` describe it, and `current_mode()` and `modes()` enumerate supported video modes. `position()` and `work_area()` use screen coordinates. `set_gamma()`, `gamma_ramp()`, and `set_gamma_ramp()` control gamma.

Hotplug changes arrive through the same event dispatchers as window events, on every window:

```cpp
window->dispatcher()->add_listener(wplatform::events::Type::monitor_connected,
	[](const wplatform::events::Event& event)
	{
		const auto* connected = event.get<wplatform::events::MonitorConnected>();
	});

window->dispatcher()->add_listener(wplatform::events::Type::monitor_disconnected,
	[](const wplatform::events::Event& event)
	{
		const auto* disconnected = event.get<wplatform::events::MonitorDisconnected>();
		// disconnected->name remains available after the monitor is gone
	});
```

All monitor queries require an initialized platform on the main thread, just like window creation.

## Joysticks

`joystick` exposes gamepads and flight sticks without leaking GLFW types. Poll the connected devices once per frame:

```cpp
#include <wplatform/joystick.hpp>

for (const auto id : wplatform::joystick::connected())
{
	if (const auto state = wplatform::joystick::state(id))
	{
		// state->axes, state->buttons, state->hats

		if (state->hat(0) & wplatform::joystick::HatUp)
		{
			// ...
		}
	}
}
```

`present(id)`, `name(id)`, and `guid(id)` describe a device, while `state(id)` returns a snapshot of its axes, buttons, and hats. `Id` covers `joy_1` through `joy_16`, and `connected()` returns just the present ones. Hat values are bit masks: `HatUp`, `HatRight`, `HatDown`, `HatLeft`, plus the combined `HatRightUp`, `HatRightDown`, `HatLeftUp`, and `HatLeftDown`. When `platform::Config::joystickHatsAsButtons` is true (the default), hat directions are also included in `state->buttons` after the regular buttons; `state->hats` is always populated.

`is_gamepad(id)`, `gamepad_name(id)`, and `gamepad_state(id)` use the standard gamepad mapping, so buttons and axes can be read by name instead of by raw index:

```cpp
if (const auto state = wplatform::joystick::gamepad_state(wplatform::joystick::Id::joy_1))
{
	if (state->down(wplatform::joystick::GamepadButton::a))
	{
		// ...
	}

	const float move = state->axis(wplatform::joystick::GamepadAxis::left_x);
}
```

Add custom SDL_GameControllerDB mappings with `update_mappings()`. Connection changes arrive on every window's dispatcher:

```cpp
window->dispatcher()->add_listener(wplatform::events::Type::joystick_connected,
	[](const wplatform::events::Event& event)
	{
		const auto* connected = event.get<wplatform::events::JoystickConnected>();
	});

window->dispatcher()->add_listener(wplatform::events::Type::joystick_disconnected,
	[](const wplatform::events::Event& event)
	{
		const auto* disconnected = event.get<wplatform::events::JoystickDisconnected>();
	});
```

Joystick state is global rather than per-window, and every query requires an initialized platform on the main thread.

## Cursors

`cursor` creates cursor images without exposing `GLFWcursor`. Standard shapes are the easiest option:

```cpp
#include <wplatform/cursor.hpp>
#include <wplatform/window.hpp>

auto cursor = wplatform::cursor::Cursor::create(
	wplatform::cursor::Shape::pointing_hand);

if (cursor && wplatform::cursor::set(*window, *cursor))
{
	// the window now uses the custom cursor
}
```

`Shape` covers `arrow`, `ibeam`, `crosshair`, `pointing_hand`, the four diagonal/horizontal/vertical `resize_*` shapes, `resize_all`, and `not_allowed`. `Cursor` is move-only and destroys its native handle when it goes out of scope, so keep it alive while the window uses it. Call `cursor::reset(*window)` to go back to the system default.

For a custom image, fill a `cursor::Image` with tightly packed RGBA8 pixels (top-left origin) and choose a hotspot inside it:

```cpp
wplatform::cursor::Image image;
image.size = { 32, 32 };
image.pixels.assign(32 * 32 * 4, 0);
// ... draw into image.pixels ...

auto cursor = wplatform::cursor::Cursor::create(image, { 0, 0 });
```

`Image::valid()` checks that the pixel buffer matches the dimensions. Creation returns `std::nullopt` for invalid images, out-of-bounds hotspots, unsupported shapes, or when the platform is not initialized on the main thread.

Cursor visibility and raw mouse motion are window settings managed through the same module:

```cpp
wplatform::cursor::set_mode(*window, wplatform::cursor::Mode::hidden);
// or Mode::normal / Mode::disabled

if (wplatform::cursor::raw_motion_supported())
	wplatform::cursor::set_raw_motion(*window, true);
```

`cursor::mode(*window)` and `cursor::raw_motion(*window)` read the current values. Raw mouse motion requires platform support and is most useful with `Mode::disabled`. As with all cursor operations, these calls must happen on the main thread while the platform is initialized.

## Clipboard

`clipboard` reads and writes the system clipboard as UTF-8 text:

```cpp
#include <wplatform/clipboard.hpp>

wplatform::clipboard::set(*window, "copied text");

if (const auto text = wplatform::clipboard::get(*window))
	std::cout << "clipboard: " << *text << '\n';
```

`set()` returns `false` if the platform is not initialized on the main thread or the window is no longer valid. `get()` returns `std::nullopt` when the clipboard is empty or its contents cannot be converted to UTF-8, so an empty but readable clipboard produces an empty string in a valid optional. GLFW uses the window handle for platform compatibility; the clipboard itself is system-wide.

## Events

Every window owns an `events::Dispatcher`. Add listeners for one event type, or without a type to receive everything, then process events once per frame with `events::poll()`:

```cpp
#include <wplatform/events.hpp>

auto* dispatcher = window->dispatcher();

dispatcher->add_listener(wplatform::events::Type::resize,
	[](const wplatform::events::Event& event)
	{
		const auto* resized = event.get<wplatform::events::Resized>();
		// resized->size.width, resized->size.height
	});

dispatcher->add_listener([](const wplatform::events::Event& event)
{
	// every event
});

while (!window->should_close())
{
	wplatform::events::poll();
	// render
}
```

An `Event` carries a `Type` and a `std::variant` payload, so `get<T>()` returns a pointer only when the payload matches:

| Type | Payload |
| --- | --- |
| `close`, `refresh` | none |
| `focus` | `FocusChanged` |
| `iconify` | `Iconified` |
| `maximize` | `Maximized` |
| `resize`, `framebuffer_resize` | `Resized`, `FramebufferResized` |
| `move` | `Moved` |
| `content_scale` | `ContentScaleChanged` |
| `key` | `KeyEvent` |
| `char_input` | `CharEvent` |
| `mouse_button` | `MouseButtonEvent` |
| `cursor_move` | `CursorMoved` |
| `cursor_enter` | `CursorEntered` |
| `scroll` | `Scrolled` |
| `drop` | `FileDropped` |
| `monitor_connected` | `MonitorConnected` |
| `monitor_disconnected` | `MonitorDisconnected` |
| `joystick_connected` | `JoystickConnected` |
| `joystick_disconnected` | `JoystickDisconnected` |

Keys, mouse buttons, actions, and modifiers are exposed as `events::Key`, `events::MouseButton`, `events::Action`, and `events::Modifiers`, so application code does not depend on GLFW constants. `add_listener` returns a `ListenerId` for `remove_listener`, `clear()` removes every listener, and `has_listeners(type)` queries the dispatcher.

`events::poll()` returns immediately, while `events::wait()` blocks until an event arrives and `events::wait(timeout)` waits at most `timeout` seconds. All three require an initialized platform on the main thread. Window state changes (resize, move, iconify, maximize) produce events; `request_close()` only sets the close flag, because GLFW reports close events from the window manager. The window uses the GLFW window user pointer to route callbacks, so avoid calling `glfwSetWindowUserPointer` on a window that uses events. A dispatcher is independent of any window, so it can also be created and dispatched to directly, which is handy for tests.

## Input

Each window also owns an `input::State` that consumes the same events, so you do not need to inspect GLFW directly:

```cpp
auto* input = window->input();

while (!window->should_close())
{
	wplatform::events::poll();

	if (input->key_pressed(wplatform::events::Key::escape))
		window->request_close();

	if (input->key_down(wplatform::events::Key::left_shift))
	{
		const auto cursor = input->cursor();
		// ...
	}

	input->new_frame();
}
```

`key_down` and `mouse_down` report whether a key or button is currently held, while `key_pressed`, `key_released`, `mouse_pressed`, and `mouse_released` are true only for the frame in which the transition happened. `cursor()` returns the latest position, `scroll_x()` and `scroll_y()` accumulate wheel deltas, and `focused()` and `cursor_inside()` track window focus and cursor entry. Call `new_frame()` once per frame after handling input to clear the one-frame deltas and scroll values; `reset()` clears everything.

`input::State` is independent of windows too: call `handle(event)` with events from any source, which makes it easy to drive from a custom event pump or a test.

Headers are opt-in: include only the modules you use, and only those need to compile in a given translation unit.
