#pragma once

#include "MikanVariantTypes.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996) // This function or variable may be unsafe
#pragma warning(disable : 4244) // conversion from 'const int64_t' to 'float', possible loss of data
#pragma warning(disable : 4715) // configuru::Config::operator[]': not all control paths return a value
#endif
#include <configuru.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

enum class eTransactionOpKind : int
{
	setProperty,
	createObject,
	destroyObject
};

/// One recorded mutation. Property values are stored in their
/// AutomationVariantText encoding so the log stays human readable and
/// undo/redo re-applies through the same text coercion the automation
/// server uses.
struct TransactionOp
{
	eTransactionOpKind kind= eTransactionOpKind::setProperty;
	std::string systemName;         // object system class name
	std::string componentClassName; // empty for a system-level property
	int componentId= -1;            // -1 targets the system itself

	// setProperty
	std::string propertyName;
	MikanVariantType valueType= MikanVariantType::INVALID;
	std::string oldValueText;
	std::string newValueText;

	// createObject / destroyObject: the component definition's JSON,
	// sufficient to recreate the object under its original component id
	configuru::Config definitionConfig;
};

/// One undoable unit: a single op, or a composite (a gesture's coalesced
/// property changes, or a destroy with its child reparent cascade).
/// Undo applies ops in reverse order, redo forward.
struct Transaction
{
	int64_t sequenceNumber= 0; // monotonic per session, never reused
	int64_t timestampMs= 0;    // epoch milliseconds
	std::string description;
	std::string gestureId; // empty when not gesture-bracketed
	std::vector<TransactionOp> ops;
};
using TransactionPtr= std::shared_ptr<Transaction>;
