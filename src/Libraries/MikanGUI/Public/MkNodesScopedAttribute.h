#pragma once

#include "MkGuiFwd.h"

class MkNodesScopedAttribute
{
public:
	// shape default = 4 (ImNodesPinShape_CircleFilled)
	explicit MkNodesScopedAttribute(int id,
	                                MkNodesAttributeType type,
	                                int shape = 4);
	~MkNodesScopedAttribute();

	MkNodesScopedAttribute(const MkNodesScopedAttribute&) = delete;
	MkNodesScopedAttribute& operator=(const MkNodesScopedAttribute&) = delete;

private:
	MkNodesAttributeType m_type;
};
