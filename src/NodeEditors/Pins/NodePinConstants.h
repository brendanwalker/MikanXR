#pragma once

#include <string>

enum class eNodePinDirection : int
{
	INVALID = -1,

	INPUT,
	OUTPUT,

	COUNT
};
extern const std::string* k_nodePinDirectionStrings;