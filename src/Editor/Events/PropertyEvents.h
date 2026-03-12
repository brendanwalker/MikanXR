#pragma once

#include "CommonConfigFwd.h"
#include <string>
#include <set>

// Forward declaration from CommonConfig.h
class ConfigPropertyChangeSet;

/// @brief Event published when a configuration property changes
struct PropertyChangedEvent
{
	/// The config object whose properties changed
	CommonConfigPtr config;

	/// Set of property names that changed
	ConfigPropertyChangeSet changeSet;

	/// Optional: Source system that triggered the change (for debugging/filtering)
	std::string sourceSystem;

	PropertyChangedEvent() = default;

	PropertyChangedEvent(
		CommonConfigPtr inConfig,
		const ConfigPropertyChangeSet& inChangeSet,
		const std::string& inSource = "")
		: config(inConfig)
		, changeSet(inChangeSet)
		, sourceSystem(inSource)
	{
	}
};
