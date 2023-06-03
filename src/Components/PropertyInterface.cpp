#include "PropertyInterface.h"

const std::string g_PropertySemanticNames[(int)ePropertySemantic::COUNT] = {
	"checkbox",
	"position",
	"rotation",
	"size3d",
	"size2d",
	"filename",
	"name",
	"anchor_id",
};
const std::string* k_PropertySemanticNames = g_PropertySemanticNames;