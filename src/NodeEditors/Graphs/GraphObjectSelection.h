#pragma once

#include <vector>

typedef int t_graph_object_id_type;

namespace GraphObjectIdType
{
	const t_graph_object_id_type NONE = -1;

	// Graph Elements
	const t_graph_object_id_type NODE = 0;
	const t_graph_object_id_type LINK = 1;

	// Assets
	const t_graph_object_id_type ASSET = 2;
	const t_graph_object_id_type VARIABLE = 3;
};

class GraphObjectSelection
{
public:
	GraphObjectSelection()= default;
	GraphObjectSelection(t_graph_object_id_type itemType, int objectIdCount);

	void clear();

	inline bool hasSelectionOfType(t_graph_object_id_type type) const { return m_objectIdType == type; }
	inline t_graph_object_id_type getObjectIdType() const { return m_objectIdType; }
	inline int getObjectCount() const { return (int)m_objectIdList.size(); }
	inline int* getRawObjectArray() { return m_objectIdList.data(); }
	
	int getObjectId(int index) const;
	void setObjectId(int index, int objectId);
	bool removeObjectId(int objectId);

private:
	t_graph_object_id_type m_objectIdType = GraphObjectIdType::NONE;
	std::vector<int> m_objectIdList;
};