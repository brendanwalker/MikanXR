#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>
#include <typeindex>
#include <unordered_map>

#include "EventBusFwd.h"

// Forward declarations
class EventBusImpl;

/// @brief Thread-safe event bus for publishing and subscribing to typed events
/// Events are dispatched immediately to all subscribers when published
class EventBus
{
public:
	EventBus();
	~EventBus();

	/// @brief Subscribe to events of a specific type
	/// @tparam EventType The event type to subscribe to
	/// @param handler Function to call when event is published
	/// @return Subscription ID (used for unsubscribing)
	template<typename EventType>
	EventBusSubID subscribe(std::function<void(const EventType&)> handler);

	/// @brief Subscribe to events of a specific type with optional filtering
	/// @tparam EventType The event type to subscribe to
	/// @param handler Function to call when event is published and passes filter
	/// @param filter Predicate function that returns true if event should be delivered to handler
	/// @return Subscription ID (used for unsubscribing)
	template<typename EventType>
	EventBusSubID subscribe(
		std::function<void(const EventType&)> handler,
		std::function<bool(const EventType&)> filter);

	/// @brief Unsubscribe from events
	/// @param subscriptionId The ID returned from subscribe()
	template<typename EventType>
	void unsubscribe(EventBusSubID subscriptionId);

	/// @brief Publish an event to all subscribers
	/// @tparam EventType The event type being published
	/// @param event The event to publish
	template<typename EventType>
	void publish(const EventType& event);

	/// @brief Check if there are any subscribers for an event type
	/// @tparam EventType The event type to check
	/// @return True if there are subscribers
	template<typename EventType>
	bool hasSubscribers() const;

private:
	std::unique_ptr<EventBusImpl> m_impl;
};

// Template implementation details
namespace EventBusDetail
{
	struct SubscriptionBase
	{
		virtual ~SubscriptionBase() = default;
		EventBusSubID id;
	};

	template<typename EventType>
	struct Subscription : public SubscriptionBase
	{
		std::function<void(const EventType&)> handler;
		std::function<bool(const EventType&)> filter; // Optional filter predicate
	};

	class EventSubscriptions
	{
	public:
		template<typename EventType>
		EventBusSubID addSubscription(
			std::function<void(const EventType&)> handler,
			std::function<bool(const EventType&)> filter = nullptr)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto subscription = std::make_unique<Subscription<EventType>>();
			subscription->id = m_nextId++;
			subscription->handler = std::move(handler);
			subscription->filter = std::move(filter);

			EventBusSubID id = subscription->id;
			m_subscriptions.push_back(std::move(subscription));

			return id;
		}

		template<typename EventType>
		void removeSubscription(EventBusSubID subscriptionId)
		{
			std::lock_guard<std::mutex> lock(m_mutex);

			auto it = std::find_if(m_subscriptions.begin(), m_subscriptions.end(),
				[subscriptionId](const auto& sub) { return sub->id == subscriptionId; });

			if (it != m_subscriptions.end())
			{
				m_subscriptions.erase(it);
			}
		}

		template<typename EventType>
		void publish(const EventType& event)
		{
			// Copy subscriptions under lock to allow safe iteration
			std::vector<std::shared_ptr<Subscription<EventType>>> handlers;
			{
				std::lock_guard<std::mutex> lock(m_mutex);

				for (const auto& sub : m_subscriptions)
				{
					if (auto typedSub = dynamic_cast<Subscription<EventType>*>(sub.get()))
					{
						auto sharedSub = std::make_shared<Subscription<EventType>>();
						sharedSub->handler = typedSub->handler;
						sharedSub->filter = typedSub->filter;
						handlers.push_back(sharedSub);
					}
				}
			}

			// Invoke handlers outside the lock (with filter check)
			for (const auto& handler : handlers)
			{
				// Check filter predicate if one exists
				if (!handler->filter || handler->filter(event))
				{
					handler->handler(event);
				}
			}
		}

		bool empty() const
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			return m_subscriptions.empty();
		}

	private:
		mutable std::mutex m_mutex;
		std::vector<std::unique_ptr<SubscriptionBase>> m_subscriptions;
		EventBusSubID m_nextId = 0;
	};
}

class EventBusImpl
{
public:
	template<typename EventType>
	int subscribe(
		std::function<void(const EventType&)> handler,
		std::function<bool(const EventType&)> filter = nullptr)
	{
		std::type_index typeIdx(typeid(EventType));
		std::lock_guard<std::mutex> lock(m_mapMutex);

		auto& subscriptions = m_eventMap[typeIdx];
		if (!subscriptions)
		{
			subscriptions = std::make_unique<EventBusDetail::EventSubscriptions>();
		}

		return subscriptions->addSubscription<EventType>(std::move(handler), std::move(filter));
	}

	template<typename EventType>
	void unsubscribe(EventBusSubID subscriptionId)
	{
		std::type_index typeIdx(typeid(EventType));
		std::lock_guard<std::mutex> lock(m_mapMutex);

		auto it = m_eventMap.find(typeIdx);
		if (it != m_eventMap.end())
		{
			it->second->removeSubscription<EventType>(subscriptionId);
		}
	}

	template<typename EventType>
	void publish(const EventType& event)
	{
		std::type_index typeIdx(typeid(EventType));

		// Get subscriptions object under lock
		std::shared_ptr<EventBusDetail::EventSubscriptions> subscriptions;
		{
			std::lock_guard<std::mutex> lock(m_mapMutex);
			auto it = m_eventMap.find(typeIdx);
			if (it != m_eventMap.end())
			{
				// Share ownership temporarily
				subscriptions = std::shared_ptr<EventBusDetail::EventSubscriptions>(
					it->second.get(), [](EventBusDetail::EventSubscriptions*) {});
			}
		}

		// Publish outside the map lock
		if (subscriptions)
		{
			subscriptions->publish<EventType>(event);
		}
	}

	template<typename EventType>
	bool hasSubscribers() const
	{
		std::type_index typeIdx(typeid(EventType));
		std::lock_guard<std::mutex> lock(m_mapMutex);

		auto it = m_eventMap.find(typeIdx);
		return it != m_eventMap.end() && !it->second->empty();
	}

private:
	mutable std::mutex m_mapMutex;
	std::unordered_map<std::type_index, std::unique_ptr<EventBusDetail::EventSubscriptions>> m_eventMap;
};

// EventBus template method implementations
template<typename EventType>
EventBusSubID EventBus::subscribe(std::function<void(const EventType&)> handler)
{
	return m_impl->subscribe<EventType>(std::move(handler), nullptr);
}

template<typename EventType>
EventBusSubID EventBus::subscribe(
	std::function<void(const EventType&)> handler,
	std::function<bool(const EventType&)> filter)
{
	return m_impl->subscribe<EventType>(std::move(handler), std::move(filter));
}

template<typename EventType>
void EventBus::unsubscribe(EventBusSubID subscriptionId)
{
	m_impl->unsubscribe<EventType>(subscriptionId);
}

template<typename EventType>
void EventBus::publish(const EventType& event)
{
	m_impl->publish<EventType>(event);
}

template<typename EventType>
bool EventBus::hasSubscribers() const
{
	return m_impl->hasSubscribers<EventType>();
}
