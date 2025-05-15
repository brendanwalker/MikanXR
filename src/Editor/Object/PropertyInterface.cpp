#include "PropertyInterface.h"

const std::string g_PropertySemanticNames[(int)ePropertySemantic::COUNT] = {
	"checkbox",
	"position",
	"rotation",
	"scale",
	"size3d",
	"size2d",
	"size1d",
	"filename",
	"name",
	"anchor_id",
	"stencil_cull_mode"
};
const std::string* k_PropertySemanticNames = g_PropertySemanticNames;

const std::string g_PropertyAttributeFileBrowseTitle= "filebrowse_title";
const std::string* k_PropertyAttributeFileBrowseTitle = &g_PropertyAttributeFileBrowseTitle;

const std::string g_PropertyAttributeFileBrowseFilter= "filebrowse_filter";
const std::string* k_PropertyAttributeFileBrowseFilter = &g_PropertyAttributeFileBrowseFilter;

const std::string g_PropertyAttributeFileBrowseFilterDesc= "filebrowse_filter_desc";
const std::string* k_PropertyAttributeFileBrowseFilterDesc = &g_PropertyAttributeFileBrowseFilterDesc;