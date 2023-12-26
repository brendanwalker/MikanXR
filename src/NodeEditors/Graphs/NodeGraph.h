#pragma once

#include "AssetFwd.h"
#include "CommonConfig.h"
#include "NodeFwd.h"
#include "Pins/NodePinConstants.h"
#include "MulticastDelegate.h"

#include <filesystem>
#include <map>
#include <string>

class NodeGraphConfig : public CommonConfig
{
public:
	NodeGraphConfig();
	NodeGraphConfig(const std::string& graphName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	bool postReadFromJSON(NodeGraphPtr graph);

	std::string className;
	int nextId= -1;

	std::vector<AssetReferenceConfigPtr> assetRefConfigs;
	std::map<t_graph_property_id, GraphPropertyConfigPtr> propertyConfigMap;
	std::vector<NodeConfigPtr> nodeConfigs;
	std::vector<NodePinConfigPtr> pinConfigs;
	std::vector<NodeLinkConfigPtr> linkConfigs;

protected:
	configuru::Config _assetRefsConfigObject;
	configuru::Config _propertiesConfigObject;
	configuru::Config _nodesConfigObject;
	configuru::Config _pinsConfigObject;
	configuru::Config _linksConfigObject;
};

class NodeGraph : public std::enable_shared_from_this<NodeGraph>
{
public:
	NodeGraph();
	virtual ~NodeGraph() {}

	virtual std::string getClassName() const { return "NodeGraph"; }

	// Generates a unique ID for each node object newly created in the editor
	int allocateId();

	// Loading
	virtual bool loadFromConfig(const NodeGraphConfig& config);
	virtual GraphPropertyPtr loadGraphPropertyFromConfig(
		GraphPropertyConfigPtr propConfig, const NodeGraphConfig& graphConfig);
	virtual AssetReferencePtr loadAssetRefFromConfig(AssetReferenceConfigPtr assetRefConfig);
	virtual NodePtr loadNodeFromConfig(NodeConfigPtr nodeConfig);
	virtual NodePinPtr loadPinFromConfig(NodePinConfigPtr pinConfig);
	virtual NodeLinkPtr loadLinkFromConfig(NodeLinkConfigPtr linkConfig);

	MulticastDelegate<void(bool success)> OnGraphLoaded;

	// Saving
	virtual void saveToConfig(NodeGraphConfig& config) const;
	virtual void saveGraphPropertyToConfig(GraphPropertyConstPtr prop, NodeGraphConfig& graphConfig) const;
	virtual void saveAssetRefToConfig(AssetReferenceConstPtr assetRef, NodeGraphConfig& graphConfig) const;
	virtual void saveNodeToConfig(NodeConstPtr node, NodeGraphConfig& graphConfig) const;
	virtual void savePinToConfig(NodePinConstPtr pin, NodeGraphConfig& graphConfig) const;
	virtual void saveLinkToConfig(NodeLinkConstPtr link, NodeGraphConfig& graphConfig) const;

	virtual void editorRender(const class NodeEditorState& editorState);

	// Assets References
	template <class t_asset_factory>
	void addAssetReferenceFactory()
	{
		auto factory = AssetReferenceFactory::createFactory<t_asset_factory>();
		std::string className = factory->getAssetRefClassName();

		m_assetRefFactories.insert({className, factory});
	}

	inline const std::map<std::string, AssetReferenceFactoryPtr>& getAssetReferenceFactories() const
	{
		return m_assetRefFactories;
	}

	AssetReferenceFactoryPtr getAssetReferenceFactory(const std::string assetRefClassName) const
	{
		auto it = m_assetRefFactories.find(assetRefClassName);

		return (it != m_assetRefFactories.end()) ? it->second : AssetReferenceFactoryPtr();
	}

	int getAssetReferenceIndex(AssetReferencePtr assetRef) const;

	inline AssetReferencePtr getAssetReferenceByIndex(int index) const
	{
		return (index >= 0 && index < (int)m_assetReferences.size()) ? m_assetReferences[index] : AssetReferencePtr();
	}

	inline const std::vector<AssetReferencePtr>& getAssetReferences() const
	{
		return m_assetReferences;
	}

	inline std::vector<AssetReferencePtr>& getAssetReferencesMutable()
	{
		return m_assetReferences;
	}

	template <class t_asset_ref_type>
	std::shared_ptr<t_asset_ref_type> addTypedAssetReference()
	{
		auto assetRef = std::make_shared<t_asset_ref_type>(shared_from_this());

		m_assetReferences.push_back(assetRef);

		if (OnAssetReferenceCreated)
			OnAssetReferenceCreated(property->getId());

		return property;
	}

	bool deleteAssetReference(AssetReferencePtr assetRef);

	MulticastDelegate<void(AssetReferencePtr assetRef)> OnAssetReferenceCreated;
	MulticastDelegate<void(AssetReferencePtr assetRef)> OnAssetReferenceDeleted;

	// Properties
	template <class t_property_factory>
	void addPropertyFactory()
	{
		auto factory = GraphPropertyFactory::createFactory<t_property_factory>();
		std::string className = factory->getGraphPropertyClassName();

		m_propertyFactories.insert({className, factory});
	}

	GraphPropertyFactoryPtr getPropertyFactory(const std::string propertyClassName) const
	{
		auto it= m_propertyFactories.find(propertyClassName);

		return (it != m_propertyFactories.end()) ? it->second : GraphPropertyFactoryPtr();
	}

	GraphPropertyPtr getPropertyById(t_graph_property_id id) const;
	GraphPropertyPtr getPropertyByName(const std::string& name) const;

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
	std::shared_ptr<t_property_type> createTypedProperty(const std::string& name)
	{
		auto property = std::make_shared<t_property_type>();
		property->setOwnerGraph(shared_from_this());
		property->setId(allocateId());
		property->setName(name);

		addProperty(property);

		return property;
	}

	void addProperty(GraphPropertyPtr property);
	bool deletePropertyById(t_graph_property_id id);

	MulticastDelegate<void(t_graph_property_id id)> OnPropertyCreated;
	MulticastDelegate<void(t_graph_property_id id)> OnPropertyModifed;
	MulticastDelegate<void(t_graph_property_id id)> OnPropertyDeleted;

	// Nodes
	virtual std::vector<NodeFactoryPtr> editorGetValidNodeFactories(const class NodeEditorState& editorState) const;

	template <class t_node_factory>
	void addNodeFactory()
	{
		auto factory = NodeFactory::createFactory<t_node_factory>();
		std::string className = factory->getNodeClassName();

		m_nodeFactories.insert({className, factory});
	}

	NodeFactoryPtr getNodeFactory(const std::string nodeClassName) const
	{
		auto it = m_nodeFactories.find(nodeClassName);

		return (it != m_nodeFactories.end()) ? it->second : NodeFactoryPtr();
	}

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

	template <class t_node_class>
	std::shared_ptr<t_node_class> createTypedNode(const NodeEditorState& nodeEditorState)
	{
		auto nodeFactory = getNodeFactory(t_node_class::k_nodeClassName);
		if (nodeFactory)
		{
			return std::static_pointer_cast<t_node_class>(createNode(nodeFactory, nodeEditorState));
		}

		return std::shared_ptr<t_node_class>();
	}

	NodePtr createNode(NodeFactoryPtr nodeFactory, const NodeEditorState& nodeEditorState);
	MulticastDelegate<void(t_node_id id)> OnNodeCreated;

	void addNode(NodePtr node);
	bool deleteNodeById(t_node_id id);
	MulticastDelegate<void(t_node_id id)> OnNodeDeleted;

	// Pins
	template <class t_pin_class, class t_pin_config_class>
	void addPinFactory()
	{
		auto factory = NodePinFactory::createFactory< TypedNodePinFactory<t_pin_class, t_pin_config_class> >();
		std::string pinClassName = factory->getPinClassName();

		m_pinFactories.insert({pinClassName, factory});
	}

	NodePinFactoryPtr getPinFactory(const std::string nodeClassName) const
	{
		auto it = m_pinFactories.find(nodeClassName);

		return (it != m_pinFactories.end()) ? it->second : NodePinFactoryPtr();
	}

	void addPin(NodePinPtr newPin);
	MulticastDelegate<void(t_node_pin_id id)> OnPinCreated;

	bool deletePinById(t_node_pin_id id);
	MulticastDelegate<void(t_node_pin_id id)> OnPinDeleted;

	// Links
	NodeLinkPtr createLink(t_node_pin_id startPinId, t_node_pin_id endPinId);
	MulticastDelegate<void(t_node_link_id id)> OnLinkCreated;

	bool deleteLinkById(t_node_link_id id);
	MulticastDelegate<void(t_node_link_id id)> OnLinkDeleted;

protected:
	// Defines all of the asset references that nodes in this graph can use
	std::map<std::string, AssetReferenceFactoryPtr> m_assetRefFactories;

	// Defines all of the property types that this graph can have
	std::map<std::string, GraphPropertyFactoryPtr> m_propertyFactories;

	// Defines all of the node types that this node graph can use
	std::map<std::string, NodeFactoryPtr> m_nodeFactories;

	// Defines all of the pin types that this node graph can use
	std::map<std::string, NodePinFactoryPtr> m_pinFactories;

	//	List of asset references used by this graph
	std::vector<AssetReferencePtr> m_assetReferences;

	// Properties assigned to this node graph
	std::map<t_graph_property_id, GraphPropertyPtr> m_properties;

	// Nodes, pins and links that make up the graph
	std::map<t_node_id, NodePtr> m_Nodes;
	std::map<t_node_pin_id, NodePinPtr> m_Pins;
	std::map<t_node_link_id, NodeLinkPtr> m_Links;

	int	m_nextId= 0;
};

class NodeGraphFactory
{
public:
	NodeGraphFactory() = default;

	inline NodeGraphConstPtr getNodeDefaultObject() const { return m_nodeGraphDefaultObject; }
	inline std::string getNodeClassName() const { return m_nodeGraphDefaultObject->getClassName(); }

	virtual NodeGraphPtr allocateNodeGraph() const;
	virtual NodeGraphPtr initialCreateNodeGraph() const;

	static NodeGraphPtr loadNodeGraph(const std::filesystem::path& path);
	static void saveNodeGraph(const std::filesystem::path& path, NodeGraphConstPtr nodeGraph);

	template <class t_node_factory_class>
	static void registerFactory()
	{
		auto factory = std::make_shared<t_node_factory_class>();

		// Create a single "NodeGraph default object" for the factory.
		// This is used to ask questions about NodeGraph without having to create one first.
		// We have to do this work outside of the NodeGraphFactory constructor,
		// because virtual functions aren't safe to call in constructor.
		factory->m_nodeGraphDefaultObject = factory->allocateNodeGraph();

		s_factoryMap.insert({factory->m_nodeGraphDefaultObject->getClassName(), factory});
	}

private:
	NodeGraphPtr m_nodeGraphDefaultObject;

	static std::map<std::string, NodeGraphFactoryPtr> s_factoryMap;
};


template <class t_node_graph_class>
class TypedNodeGraphFactory : public NodeGraphFactory
{
public:
	TypedNodeGraphFactory() = default;

	virtual NodeGraphPtr allocateNodeGraph() const override
	{
		return std::make_shared<t_node_graph_class>();
	}
};