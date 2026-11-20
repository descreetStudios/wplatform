#include <wplatform/events.hpp>
#include <wplatform/platform.hpp>

#include <wcore/log.hpp>

#include "internal.hpp"

#include <eventpp/callbacklist.h>
#include <eventpp/eventdispatcher.h>

#include <GLFW/glfw3.h>

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace wplatform::events
{
	namespace
	{
		struct EventTypePolicy
		{
			static Type getEvent(const Event& event) noexcept
			{
				return event.type;
			}
		};

		using TypeDispatcher = eventpp::EventDispatcher<Type, void(const Event&), EventTypePolicy>;
		using AnyDispatcher = eventpp::CallbackList<void(const Event&)>;

		std::vector<std::weak_ptr<Dispatcher>> gDispatchers;
	} // namespace

	struct Dispatcher::Impl
	{
		struct Listener
		{
			bool any{ false };
			Type type{ Type::close };
			TypeDispatcher::Handle typeHandle{};
			AnyDispatcher::Handle anyHandle{};
		};

		TypeDispatcher byType;
		AnyDispatcher any;
		std::unordered_map<ListenerId, Listener> listeners;
		ListenerId next{ 1 };
	};

	Dispatcher::Dispatcher()
		: mImpl{ std::make_unique<Impl>() }
	{
	}

	Dispatcher::~Dispatcher() = default;
	Dispatcher::Dispatcher(Dispatcher&&) noexcept = default;
	Dispatcher& Dispatcher::operator=(Dispatcher&&) noexcept = default;

	Dispatcher::ListenerId Dispatcher::add_listener(Type type, Callback callback)
	{
		const ListenerId id{ mImpl->next++ };

		Impl::Listener listener;
		listener.type = type;
		listener.typeHandle = mImpl->byType.appendListener(type, callback);
		mImpl->listeners.emplace(id, std::move(listener));

		return id;
	}

	Dispatcher::ListenerId Dispatcher::add_listener(Callback callback)
	{
		const ListenerId id{ mImpl->next++ };

		Impl::Listener listener;
		listener.any = true;
		listener.anyHandle = mImpl->any.append(callback);
		mImpl->listeners.emplace(id, std::move(listener));

		return id;
	}

	bool Dispatcher::remove_listener(ListenerId id)
	{
		const auto it{ mImpl->listeners.find(id) };
		if (it == mImpl->listeners.end())
			return false;

		if (it->second.any)
			mImpl->any.remove(it->second.anyHandle);
		else
			mImpl->byType.removeListener(it->second.type, it->second.typeHandle);

		mImpl->listeners.erase(it);
		return true;
	}

	void Dispatcher::clear()
	{
		for (const auto& entry : mImpl->listeners)
		{
			const Impl::Listener& listener{ entry.second };
			if (listener.any)
				mImpl->any.remove(listener.anyHandle);
			else
				mImpl->byType.removeListener(listener.type, listener.typeHandle);
		}

		mImpl->listeners.clear();
	}

	void Dispatcher::dispatch(const Event& event)
	{
		mImpl->byType.dispatch(event);
		mImpl->any(event);
	}

	bool Dispatcher::has_listeners(Type type) const noexcept
	{
		return mImpl->byType.hasAnyListener(type) || !mImpl->any.empty();
	}

	std::size_t Dispatcher::listener_count() const noexcept
	{
		return mImpl->listeners.size();
	}

	namespace detail
	{
		void register_dispatcher(const std::shared_ptr<Dispatcher>& dispatcher)
		{
			if (dispatcher)
				gDispatchers.emplace_back(dispatcher);
		}

		void broadcast(const Event& event)
		{
			for (auto it = gDispatchers.begin(); it != gDispatchers.end();)
			{
				if (it->expired())
					it = gDispatchers.erase(it);
				else
					++it;
			}

			const std::vector<std::weak_ptr<Dispatcher>> snapshot{ gDispatchers };
			for (const auto& entry : snapshot)
			{
				if (const auto dispatcher{ entry.lock() })
					dispatcher->dispatch(event);
			}
		}
	} // namespace detail

	namespace
	{
		bool can_process_events() noexcept
		{
			if (!platform::initialized() || !platform::is_main_thread())
			{
				WCORE_ERROR("wplatform: event processing requires an initialized platform on the main thread");
				return false;
			}

			return true;
		}
	} // namespace

	bool poll() noexcept
	{
		if (!can_process_events())
			return false;

		glfwPollEvents();
		return true;
	}

	bool wait() noexcept
	{
		if (!can_process_events())
			return false;

		glfwWaitEvents();
		return true;
	}

	bool wait(f64 timeout) noexcept
	{
		if (!can_process_events())
			return false;

		glfwWaitEventsTimeout(timeout);
		return true;
	}
} // namespace wplatform::events
