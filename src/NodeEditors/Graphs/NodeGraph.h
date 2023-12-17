#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "NodeFwd.h"
#include "MulticastDelegate.h"

#include <filesystem>
#include <map>
#include <string>

class NodeGraphFactory
{
public:
	NodeGraphFactory() = default;

	static NodeGraphPtr loadNodeGraph(const std::filesystem::path& path);
	static void saveNodeGraph(const std::filesystem::path& path, NodeGraphConstPtr nodeGraph);

	template <class t_node_factory_class>
	static void registerFactory()
	{
		auto factory = std::make_shared<t_node_factory_class>();
		const std::string typeName = typeid(t_node_factory_class).name();

		s_factoryMap.insert({typeName, factory});
	}

protected:
	virtual NodeGraphPtr allocateNodeGraph() const;

private:
	static std::map<std::string, NodeGraphFactoryPtr> s_factoryMap;
};

class NodeGraphConfig : public CommonConfig
{
public:
	NodeGraphConfig();
	NodeGraphConfig(const std::string& graphName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	inline void setNodeGraphClassName(const std::string& className) { m_className = className; }
	inline const std::string& getNodeGraphClassName() const { return m_className; }

protected:
	std::string m_className;
};

class NodeGraph : public std::enable_shared_from_this<NodeGraph>
{
public:
	NodeGraph();
	virtual ~NodeGraph() {}

	virtual bool loadFromConfig(const class NodeGraphConfig& config);
	virtual void saveToConfig(class NodeGraphConfig& config) const;

	inline float getTimeInSeconds() const { return m_timeInSeconds; }

	GraphPropertyPtr getPropertyById(t_graph_property_id id) const;
	GraphPropertyPtr getPropertyByName(const std::string& name) const;

	inline const std::vector<AssetReferenceFactoryPtr>& getAssetReferenceFactories() const
	{
		return m_assetRefFactories;
	}

	inline const std::map<t_graph_property_id, GraphPropertyPtr>& getPropertyMap() const
	{
		return m_properties;
	}

	template <class t_property_type>
	std::shared_ptr<t_property_type> getTypedPropertyById(t_graph_property_id id) const
	{
		return std::dynamic_pointer_cast<t_property_type>(getPropertyById(id));
	}

	template <class t_property_type>
	std::shared_ptr<t_property_type> getTypedPropertyByName(const std::string& name) const
	{
		return std::dynamic_pointer_cast<t_property_type>(getPropertyByName(name));
	}

	template <class t_property_type>
	std::shared_ptr<t_property_type> addTypedProperty(const std::string& name)
	{
		auto property = std::make_shared<t_property_type>(shared_from_this());
		property->setName(name);

		m_properties.insert({property->getId(), property});

		if (OnPropertyCreated)
			OnPropertyCreated(property->getId());

		return property;
	}

	bool deletePropertyById(t_graph_property_id id);

	MulticastDelegate<void(t_graph_property_id id)> OnPropertyCreated;
	MulticastDelegate<void(t_graph_property_id id)> OnPropertyModifed;
	MulticastDelegate<void(t_graph_property_id id)> OnPropertyDeleted;

	template <class _Pr>
	NodePtr getNodeByPredicate(_Pr predicate) const
	{
		auto it = std::find_if(m_Nodes.begin(), m_Nodes.end(), predicate);
		if (it != m_Nodes.end())
		{
			return it->second;
		}

		return NodePtr();
	}
	NodePtr getNodeById(t_node_id id) const;
	NodePtr getEventNodeByName(const std::string& eventName) const;

	NodePinPtr getNodePinById(t_node_pin_id id) const;
	NodeLinkPtr getNodeLinkById(t_node_link_id id) const;

	virtual void update(class NodeEvaluator& evaluator);

	NodePtr createNode(NodeFactoryPtr nodeFactory, const NodeEditorState* nodeEditorState);
	MulticastDelegate<void(t_node_id id)> OnNodeCreated;

	bool deleteNodeById(t_node_id id);
	MulticastDelegate<void(t_node_id id)> OnNodeDeleted;

	bool deletePinById(t_node_pin_id id);
	MulticastDelegate<void(t_node_pin_id id)> OnPinDeleted;

	NodeLinkPtr createLink(t_node_pin_id startPinId, t_node_pin_id endPinId);
	MulticastDelegate<void(t_node_link_id id)> OnLinkCreated;

	bool deleteLinkById(t_node_link_id id);
	MulticastDelegate<void(t_node_link_id id)> OnLinkDeleted;

	virtual std::vector<NodeFactoryPtr> editorGetValidNodeFactories(const class NodeEditorState& editorState) const;
	virtual void editorRender(const class NodeEditorState& editorState);

protected:
	int allocateId();

	float m_timeInSeconds;

	// Defines all of the asset references that this node graph can use
	std::vector<AssetReferenceFactoryPtr> m_assetRefFactories;

	// Defines all of the node types that this node graph can use
	std::vector<NodeFactoryPtr> m_nodeFactories;

	// Properties assigned to this node graph
	// * GraphAssetListProperty - List of all asset references used by this graph
	// * GraphVariableList - Lists of graph variables (models, textures, materials, ...)
	std::map<t_graph_property_id, GraphPropertyPtr> m_properties;

	// Nodes, pins and links that make up the graph
	std::map<t_node_id, NodePtr> m_Nodes;
	std::map<t_node_pin_id, NodePinPtr> m_Pins;
	std::map<t_node_link_id, NodeLinkPtr> m_Links;

	int	m_nextId= 0;

	friend class Node;
	friend class NodeLink;
	friend class NodePin;
	friend class GraphProperty;
};