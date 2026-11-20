#pragma once

#include <memory>

#include <wplatform/events.hpp>

namespace wplatform::events::detail
{
	void register_dispatcher(const std::shared_ptr<Dispatcher>& dispatcher);
	void broadcast(const Event& event);
} // namespace wplatform::events::detail

namespace wplatform::monitor::detail
{
	void install_callback() noexcept;
} // namespace wplatform::monitor::detail

namespace wplatform::joystick::detail
{
	void install_callback() noexcept;
} // namespace wplatform::joystick::detail
