# MikanXR Event Bus

The EventBus provides a centralized, type-safe event routing system for decoupling components.

## Architecture

- **EventBus**: Thread-safe publish/subscribe event dispatcher
- **PropertyChangedEvent**: Event for configuration property changes
- Property events are **dual-dispatched**: both via legacy `MulticastDelegate` AND the EventBus

## Usage

### Subscribing to Property Changes (No Filter)

```cpp
#include "EventBus.h"
#include "PropertyEvents.h"
#include "App.h"

// Get the global event bus
EventBus* eventBus = App::getInstance()->getEventBus();

// Subscribe to ALL property change events
int subscriptionId = eventBus->subscribe<PropertyChangedEvent>(
    [](const PropertyChangedEvent& event) {
        // Access the changed config
        CommonConfigPtr config = event.config;

        // Check which properties changed
        if (event.changeSet.hasPropertyName("myPropertyName"))
        {
            // React to specific property change
        }
    });
```

### Subscribing with Predicate Filters

Filters are evaluated **during dispatch** - handlers are only invoked if the filter returns true:

```cpp
// Filter by specific config object
CommonConfigPtr myConfig = getMyConfig();
int subId = eventBus->subscribe<PropertyChangedEvent>(
    [](const PropertyChangedEvent& event) {
        // Handle event - only called for myConfig changes
    },
    [myConfig](const PropertyChangedEvent& event) {
        return event.config == myConfig; // Filter predicate
    });

// Filter by config type/name
int subId = eventBus->subscribe<PropertyChangedEvent>(
    [](const PropertyChangedEvent& event) {
        // Handle VideoSourceConfig changes only
    },
    [](const PropertyChangedEvent& event) {
        return event.config->getConfigName() == "VideoSourceConfig";
    });

// Filter by specific property name
int subId = eventBus->subscribe<PropertyChangedEvent>(
    [](const PropertyChangedEvent& event) {
        // Handle frameRate changes only
    },
    [](const PropertyChangedEvent& event) {
        return event.changeSet.hasPropertyName("frameRate");
    });

// Combine multiple filters with logical operators
int subId = eventBus->subscribe<PropertyChangedEvent>(
    [](const PropertyChangedEvent& event) {
        // Handle specific config AND specific property
    },
    [myConfig](const PropertyChangedEvent& event) {
        return event.config == myConfig &&
               event.changeSet.hasPropertyName("frameRate");
    });

// Filter by source system (for debugging/tracking)
int subId = eventBus->subscribe<PropertyChangedEvent>(
    [](const PropertyChangedEvent& event) {
        // Handle only events from UI
    },
    [](const PropertyChangedEvent& event) {
        return event.sourceSystem == "UIEditor";
    });
```

### Unsubscribing

```cpp
eventBus->unsubscribe<PropertyChangedEvent>(subscriptionId);
```

### Publishing Events (Automatic for Properties)

Property changes are automatically published when `CommonConfig::notifyPropertyChanged()` is called:

```cpp
ConfigPropertyChangeSet changeSet;
changeSet.addPropertyName("videoFrameQueueSize");
notifyPropertyChanged(changeSet);
// Event is automatically published to both delegates AND event bus
```

### Creating Custom Events

```cpp
// Define your event struct
struct CustomEvent {
    int entityId;
    std::string message;
};

// Publish
eventBus->publish(CustomEvent{42, "Hello"});

// Subscribe (no filter)
eventBus->subscribe<CustomEvent>([](const CustomEvent& event) {
    // Handle all custom events
});

// Subscribe with filter
eventBus->subscribe<CustomEvent>(
    [](const CustomEvent& event) {
        // Handle specific entity events only
    },
    [](const CustomEvent& event) {
        return event.entityId == 42; // Filter by entity ID
    });
```

## Filter Design Patterns

### Pattern 1: Single-Object Observer
```cpp
// Component only cares about its own config
class MyComponent {
    CommonConfigPtr m_config;
    int m_subscriptionId;

    void init() {
        auto bus = App::getInstance()->getEventBus();
        m_subscriptionId = bus->subscribe<PropertyChangedEvent>(
            [this](const PropertyChangedEvent& e) {
                this->onMyConfigChanged(e);
            },
            [this](const PropertyChangedEvent& e) {
                return e.config == m_config; // Only my config
            });
    }
};
```

### Pattern 2: Property-Specific Observer
```cpp
// Only care about specific property across all configs
class FrameRateMonitor {
    void init() {
        auto bus = App::getInstance()->getEventBus();
        bus->subscribe<PropertyChangedEvent>(
            [this](const PropertyChangedEvent& e) {
                this->onFrameRateChanged(e.config);
            },
            [](const PropertyChangedEvent& e) {
                return e.changeSet.hasPropertyName("frameRate");
            });
    }
};
```

### Pattern 3: Config Type Observer
```cpp
// Monitor all configs of a specific type
class VideoSourceManager {
    void init() {
        auto bus = App::getInstance()->getEventBus();
        bus->subscribe<PropertyChangedEvent>(
            [this](const PropertyChangedEvent& e) {
                this->onVideoSourceConfigChanged(e);
            },
            [](const PropertyChangedEvent& e) {
                auto videoSourceConfig =
                    std::dynamic_pointer_cast<VideoSourceConfig>(e.config);
                return videoSourceConfig != nullptr;
            });
    }
};
```

### Pattern 4: Multi-Property Observer
```cpp
// Monitor multiple related properties
class CameraIntrinsicsObserver {
    void init() {
        auto bus = App::getInstance()->getEventBus();
        bus->subscribe<PropertyChangedEvent>(
            [this](const PropertyChangedEvent& e) {
                this->onIntrinsicsChanged(e);
            },
            [](const PropertyChangedEvent& e) {
                return e.changeSet.hasPropertyName("focalLength") ||
                       e.changeSet.hasPropertyName("principalPoint") ||
                       e.changeSet.hasPropertyName("distortionCoefficients");
            });
    }
};
```

## Benefits vs Delegates

### Use Delegates When:
- High-frequency events (frame updates, VR tracking)
- Performance-critical paths
- Tight coupling is acceptable
- Few subscribers needed

### Use EventBus When:
- Cross-cutting concerns (property changes, lifecycle events)
- Many potential subscribers
- Decoupling systems
- Central logging/debugging needed
- Plugin-style architecture
- Need filtering by event properties

## Migration Strategy

Current implementation uses **dual-dispatch** for backward compatibility:
1. All property changes fire both delegate AND event bus
2. Existing code using delegates continues to work
3. New code can use EventBus for better decoupling
4. Gradually migrate subscribers from delegates to EventBus

## Thread Safety

EventBus is fully thread-safe:
- Subscribers can be added/removed from any thread
- Events can be published from any thread
- Handlers are invoked **synchronously** on the publishing thread

## Performance Considerations

- **Immediate dispatch** (not queued) - events delivered synchronously
- **Handler invocation outside locks** - safe for handlers to modify subscriptions
- **Type-based dispatch** using `std::type_index` - O(1) lookup
- **Filter evaluation during dispatch** - handlers not invoked if filter returns false
- **Minimal overhead** for unused event types (no allocation until first subscriber)
- **Filter overhead** - predicate functions are called for each subscriber, keep them simple
  - ✅ Good: Pointer comparison (`e.config == myConfig`)
  - ✅ Good: String comparison (`e.config->getConfigName() == "Foo"`)
  - ✅ Good: Set lookup (`e.changeSet.hasPropertyName("bar")`)
  - ⚠️ Avoid: Heavy computation in filters (defer to handler)

## Filter Best Practices

1. **Keep filters fast** - They're evaluated on every publish for every subscriber
2. **Capture by value for safety** - `[myConfig]` not `[&myConfig]` to avoid dangling refs
3. **Use filters for structural filtering** - Config identity, property names, types
4. **Use handler logic for value filtering** - Complex conditions on property values
5. **Combine filters with &&/||** - Single predicate is more efficient than chaining
