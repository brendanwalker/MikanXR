#pragma once

#include "MkGuiExport.h"
#include "MkGuiFwd.h"

class MIKAN_GUI_CLASS MkNodesScopedAttribute
{
public:
	// shape default = 4 (ImNodesPinShape_CircleFilled)
	explicit MkNodesScopedAttribute(int id,
									MkNodesAttributeType type,
									int shape= 4);
	~MkNodesScopedAttribute();

	MkNodesScopedAttribute(const MkNodesScopedAttribute&)= delete;
	MkNodesScopedAttribute& operator=(const MkNodesScopedAttribute&)= delete;

private:
	MkNodesAttributeType m_type;
};
