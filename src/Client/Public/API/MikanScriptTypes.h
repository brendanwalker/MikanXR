#pragma once

#include "MikanEventTypes.h"

// Script Event Types
struct MikanScriptMessageInfo : public MikanEvent
{
	inline static const std::string k_typeName = "MikanScriptMessageInfo";

	std::string content;

	MikanScriptMessageInfo() : MikanEvent(k_typeName) {}
};