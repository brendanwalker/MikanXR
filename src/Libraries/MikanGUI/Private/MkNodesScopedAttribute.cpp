#include "MkNodesScopedAttribute.h"

#include "imnodes.h"

MkNodesScopedAttribute::MkNodesScopedAttribute(int id, MkNodesAttributeType type, int shape)
	: m_type(type)
{
	switch (m_type)
	{
		case MkNodesAttributeType::Input:
			ImNodes::BeginInputAttribute(id, static_cast<ImNodesPinShape>(shape));
			break;
		case MkNodesAttributeType::Output:
			ImNodes::BeginOutputAttribute(id, static_cast<ImNodesPinShape>(shape));
			break;
		case MkNodesAttributeType::Static:
			ImNodes::BeginStaticAttribute(id);
			break;
	}
}

MkNodesScopedAttribute::~MkNodesScopedAttribute()
{
	switch (m_type)
	{
		case MkNodesAttributeType::Input:
			ImNodes::EndInputAttribute();
			break;
		case MkNodesAttributeType::Output:
			ImNodes::EndOutputAttribute();
			break;
		case MkNodesAttributeType::Static:
			ImNodes::EndStaticAttribute();
			break;
	}
}
