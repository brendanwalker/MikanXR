#include "GraphObjectSelection.h"

GraphObjectSelection::GraphObjectSelection(
	t_graph_object_id_type itemType, 
	int objectIdCount)
	: m_objectIdType(itemType)
{
	for(int i= 0; i < objectIdCount; i++)
		m_objectIdList.push_back(-1);
}

void GraphObjectSelection::clear()
{
	m_objectIdType = GraphObjectIdType::NONE;
	m_objectIdList.clear();
}

int GraphObjectSelection::getObjectId(int index) const 
{ 
	return (index >= 0 && index < getObjectCount()) ? m_objectIdList[index] : -1; 
}

void GraphObjectSelection::setObjectId(int index, int objectId)
{
	if (index >= 0 && index < getObjectCount())
	{
		m_objectIdList[index]= objectId;
	}
}

bool GraphObjectSelection::removeObjectId(int objectId)
{
	auto it= std::find(m_objectIdList.begin(), m_objectIdList.end(), objectId);
	if (it != m_objectIdList.end())
	{
		m_objectIdList.erase(it);
		if (m_objectIdList.size() == 0)
		{
			m_objectIdType= GraphObjectIdType::NONE;
		}

		return true;
	}

	return false;
}