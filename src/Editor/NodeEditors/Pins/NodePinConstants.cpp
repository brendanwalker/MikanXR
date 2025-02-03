#include "NodePinConstants.h"

const std::string g_nodePinDirectionStrings[(int)eNodePinDirection::COUNT] = {
	"input",
	"output",
};
extern const std::string* k_nodePinDirectionStrings= g_nodePinDirectionStrings;