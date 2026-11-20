<div align="center">

# wplatform

**A C++20 platform foundation for modern projects.**

<a href="https://isocpp.org/"><img src="https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white" alt="C++20"></a>
<a href="https://cmake.org/"><img src="https://img.shields.io/badge/CMake-3.24%2B-064F8C?logo=cmake&logoColor=white" alt="CMake 3.24+"></a>

</div>

`wplatform` is a small platform library for C++ projects. It wraps GLFW behind a clean C++20 API: a static library, a single CMake target (`wplatform::wplatform`), and headers under `<wplatform/...>`.

Today it provides platform initialization and backend selection, a move-only RAII window type with an event dispatcher, and a version header generated from the CMake project version. The target also carries its dependencies (GLFW, `wcore`, and eventpp), so consumers do not need to fetch them separately.

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
#include <wplatform/events.hpp>
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

Keys, mouse buttons, actions, and modifiers are exposed as `events::Key`, `events::MouseButton`, `events::Action`, and `events::Modifiers`, so application code does not depend on GLFW constants. `add_listener` returns a `ListenerId` for `remove_listener`, `clear()` removes every listener, and `has_listeners(type)` queries the dispatcher.

`events::poll()` returns immediately, while `events::wait()` blocks until an event arrives and `events::wait(timeout)` waits at most `timeout` seconds. All three require an initialized platform on the main thread. Window state changes (resize, move, iconify, maximize) produce events; `request_close()` only sets the close flag, because GLFW reports close events from the window manager. The dispatcher stores itself in the GLFW window user pointer, so avoid calling `glfwSetWindowUserPointer` on a window that uses events. A dispatcher is independent of any window, so it can also be created and dispatched to directly, which is handy for tests.

Headers are opt-in: include only the modules you use, and only those need to compile in a given translation unit.
