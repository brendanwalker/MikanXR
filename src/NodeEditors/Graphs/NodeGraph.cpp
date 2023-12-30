#include "NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Logger.h"
#include "AssetReference.h"
#include "Nodes/Node.h"
#include "Nodes/EventNode.h"
#include "Pins/ArrayPin.h"
#include "Pins/FloatPin.h"
#include "Pins/FlowPin.h"
#include "Pins/IntPin.h"
#include "Pins/NodeLink.h"
#include "Pins/NodePin.h"
#include "Pins/PropertyPin.h"
#include "Pins/TexturePin.h"
#include "Properties/GraphArrayProperty.h"
#include "Properties/GraphVariableList.h"
#include "NodeEditorState.h"

#include "imnodes.h"

#include <filesystem>
#include <functional>

// -- NodeGraphConfig -----
NodeGraphConfig::NodeGraphConfig() : CommonConfig() 
{
}

NodeGraphConfig::NodeGraphConfig(const std::string& graphName) : CommonConfig(graphName) 
{
}

configuru::Config NodeGraphConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["class_name"] = className;
	pt["next_id"] = nextId;

	// Write out propertyConfigMap as an array
	{
		auto configArray = configuru::Config::array();
		for (auto it = propertyConfigMap.begin(); it != propertyConfigMap.end(); it++)
		{
			configArray.push_back(it->second->writeToJSON());
		}
		pt["properties"] = configArray;
	}

	writeStdConfigVector(pt, "assetReferences", assetRefConfigs);
	writeStdConfigVector(pt, "nodes", nodeConfigs);
	writeStdConfigVector(pt, "pins", pinConfigs);
	writeStdConfigVector(pt, "links", linkConfigs);

	return pt;
}

void NodeGraphConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	className = pt.get_or<std::string>("class_name", "NodeGraph");
	nextId = pt.get_or<int>("next_id", -1);

	// These get evaluated in postReadFromJSON after we use className above
	// to allocate a node graph that has the factories to process these config objects
	_assetRefsConfigObject= pt["assetReferences"];
	_propertiesConfigObject= pt["properties"];
	_nodesConfigObject= pt["nodes"];
	_pinsConfigObject= pt["pins"];

	// Links don't need need any special class_name based construction
	// so we can just load the link configs the simple way
	readStdConfigVector(pt, "links", linkConfigs);
}

template<class t_object_type>
static bool readNodeGraphConfigArray(
	const configuru::Config& arrayConfigObject,
	const std::string& arrayName,
	std::vector< std::shared_ptr<t_object_type> >& vector,
	std::function<CommonConfigPtr(const std::string& className)> allocateConfig)
{
	const auto& configArray = arrayConfigObject.as_array();
	bool success= true;

	vector.clear();
	for (auto it = configArray.begin(); it != configArray.end(); it++)
	{
		// Each config block should have a class name we can use to look up the factory
		// that we can use to allocate the correct config object
		const std::string className = it->get_or<std::string>("class_name", "");
		if (className.empty())
		{
			MIKAN_LOG_ERROR("readNodeGraphConfigVector") << "Config entry missing class name in array: " << arrayName;
			success= false;
			continue;
		}

		CommonConfigPtr config = allocateConfig(className);
		if (!config)
		{
			MIKAN_LOG_ERROR("readNodeGraphConfigVector") << "Failed to allocate config for class name: " << className << ", in array: " << arrayName;
			success= false;
			continue;
		}

		auto typedConfig = std::static_pointer_cast<t_object_type>(config);
		typedConfig->readFromJSON(*it);

		vector.push_back(typedConfig);
	}

	return success;
}

bool NodeGraphConfig::postReadFromJSON(NodeGraphPtr graph)
{
	std::vector<GraphPropertyConfigPtr> propertyConfigs;
	bool success= true;

	// Use factories to create the config of the appropriate type
	success&= readNodeGraphConfigArray(
		_assetRefsConfigObject, "assetReferences", assetRefConfigs,
		[graph](const std::string& className) {
			return graph->getAssetReferenceFactory(className)->allocateAssetReferenceConfig();
		});
	success&= readNodeGraphConfigArray(
		_propertiesConfigObject, "properties", propertyConfigs,
		[graph](const std::string& className) {
			return graph->getPropertyFactory(className)->allocatePropertyConfig();
		});
	success&= readNodeGraphConfigArray(
		_nodesConfigObject, "nodes", nodeConfigs,
		[graph](const std::string& className) {
			return graph->getNodeFactory(className)->allocateNodeConfig();
		});
	success&= readNodeGraphConfigArray(
		_pinsConfigObject, "pins", pinConfigs,
		[graph](const std::string& className) {
			return graph->getPinFactory(className)->allocatePinConfig();
		});

	// For graph properties, we actually need to use a map
	// so that we can look up property config by id
	for (GraphPropertyConfigPtr propertyConfig : propertyConfigs)
	{
		propertyConfigMap.insert({propertyConfig->id, propertyConfig});
	}

	return success;
}

// -- NodeGraph -----
NodeGraph::NodeGraph()
{
	// Add pin types nodes in this graph can use
	addPinFactory<ArrayPin, ArrayPinConfig>();
	addPinFactory<FlowPin, NodePinConfig>();
	addPinFactory<FloatPin, NodePinConfig>();
	addPinFactory<Float2Pin, NodePinConfig>();
	addPinFactory<Float3Pin, NodePinConfig>();
	addPinFactory<Float4Pin, NodePinConfig>();
	addPinFactory<IntPin, NodePinConfig>();
	addPinFactory<Int2Pin, NodePinConfig>();
	addPinFactory<Int3Pin, NodePinConfig>();
	addPinFactory<Int4Pin, NodePinConfig>();
	addPinFactory<NodePin, NodePinConfig>();
	addPinFactory<PropertyPin, PropertyPinConfig>();
	addPinFactory<TexturePin, NodePinConfig>();

	// Add property types this graph can use
	addPropertyFactory<GraphArrayPropertyFactory>();
	addPropertyFactory<GraphVariableListFactory>();
}

NodeGraph::~NodeGraph()
{
	disposeResources();
}

bool NodeGraph::loadFromConfig(const NodeGraphConfig& config)
{
	bool bSuccess= true;

	m_nextId= config.nextId;

	// Load all asset references (no dependencies)
	for (auto assetRefConfig : config.assetRefConfigs)
	{
		bSuccess&= loadAssetRefFromConfig(assetRefConfig);
	}

	// Load all properties (depends on asset references)
	for (auto it = config.propertyConfigMap.begin(); it != config.propertyConfigMap.end(); it++)
	{
		bSuccess&= (loadGraphPropertyFromConfig(it->second, config) != nullptr);
	}

	// Allocate all nodes, pins and links first (delay the setup)
	for (auto nodeConfig : config.nodeConfigs)
	{
		bSuccess&= allocateNodeFromConfig(nodeConfig);
	}
	for (auto pinConfig : config.pinConfigs)
	{
		bSuccess&= allocatePinFromConfig(pinConfig);
	}
	for (auto linkConfig : config.linkConfigs)
	{
		bSuccess&= allocateLinkFromConfig(linkConfig);
	}

	// Setup all nodes, pins and links new that they can safely resolve circular references to each other
	for (auto nodeConfig : config.nodeConfigs)
	{
		bSuccess&= loadNodeFromConfig(nodeConfig);
	}
	for (auto pinConfig : config.pinConfigs)
	{
		bSuccess&= loadPinFromConfig(pinConfig);
	}
	for (auto linkConfig : config.linkConfigs)
	{
		bSuccess&= loadLinkFromConfig(linkConfig);
	}

	if (bSuccess)
	{
		bSuccess&= createResources();
	}

	if (OnGraphLoaded)
	{
		OnGraphLoaded(bSuccess);
	}

	return bSuccess;
}

bool NodeGraph::loadAssetRefFromConfig(AssetReferenceConfigPtr assetRefConfig)
{
	AssetReferenceFactoryPtr factory = getAssetReferenceFactory(assetRefConfig->className);

	if (factory)
	{
		AssetReferencePtr assetRef = factory->allocateAssetReference();
		if (assetRef)
		{
			if (assetRef->loadFromConfig(assetRefConfig))
			{
				m_assetReferences.push_back(assetRef);
				return true;
			}
			else
			{
				MIKAN_LOG_INFO("NodeGraph::loadAssetRefFromConfig") << "Config parse failure";
			}
		}
		else
		{
			MIKAN_LOG_INFO("NodeGraph::loadAssetRefFromConfig") << "AssetReference allocation failure";
		}
	}
	else
	{
		MIKAN_LOG_INFO("NodeGraph::loadAssetRefFromConfig") << "Missing factory";
	}

	MIKAN_LOG_INFO("NodeGraph::loadAssetRefFromConfig")
		<< "Failed to load AssetReference from config"
		<< ", class: " << assetRefConfig->className
		<< ", path: " << assetRefConfig->assetPath;

	return false;
}

GraphPropertyPtr NodeGraph::loadGraphPropertyFromConfig(
	GraphPropertyConfigPtr propConfig, 
	const NodeGraphConfig& graphConfig)
{
	GraphPropertyPtr property= getPropertyById(propConfig->id);

	// Skip properties that were already created by a parent property (i.e. ArrayProperty)
	if (property)
	{
		assert(propConfig->parentId != -1);
		assert(getPropertyById(propConfig->parentId));
		return property;
	}

	GraphPropertyFactoryPtr factory = getPropertyFactory(propConfig->className);
	if (factory)
	{
		property = factory->allocateProperty();
		if (property)
		{
			property->setOwnerGraph(shared_from_this());

			if (property->loadFromConfig(propConfig, graphConfig))
			{
				m_properties.insert({property->getId(), property});
				return property;
			}
			else
			{
				MIKAN_LOG_INFO("NodeGraph::loadGraphPropertyFromConfig") << "Config parse failure";
			}
		}
		else
		{
			MIKAN_LOG_INFO("NodeGraph::loadGraphPropertyFromConfig") << "GraphProperty allocation failure";
		}
	}
	else
	{
		MIKAN_LOG_INFO("NodeGraph::loadGraphPropertyFromConfig") << "Missing factory";
	}

	MIKAN_LOG_INFO("NodeGraph::loadGraphPropertyFromConfig")
		<< "Failed to load GraphProperty from config"
		<< ", property id: " << propConfig->id
		<< ", parent property id: " << propConfig->parentId
		<< ", property class: " << propConfig->className;

	return GraphPropertyPtr();
}

bool NodeGraph::allocateNodeFromConfig(NodeConfigPtr nodeConfig)
{
	NodeFactoryPtr factory = getNodeFactory(nodeConfig->className);

	if (factory)
	{
		NodePtr node = factory->allocateNode();
		if (node)
		{
			node->setOwnerGraph(shared_from_this());
			m_Nodes.insert({nodeConfig->id, node});

			return true;
		}
		else
		{
			MIKAN_LOG_INFO("NodeGraph::allocateNodeFromConfig") << "Node allocation failure";
		}
	}
	else
	{
		MIKAN_LOG_INFO("NodeGraph::allocateNodeFromConfig") << "Missing factory";
	}

	MIKAN_LOG_INFO("NodeGraph::allocateNodeFromConfig")
		<< "Failed to load node from config"
		<< ", node id: " << nodeConfig->id
		<< ", node class: " << nodeConfig->className;

	return false;
}

bool NodeGraph::loadNodeFromConfig(NodeConfigPtr nodeConfig)
{
	NodePtr node = getNodeById(nodeConfig->id);
	assert (node);

	if (!node->loadFromConfig(nodeConfig))
	{
		MIKAN_LOG_INFO("NodeGraph::loadNodeFromConfig")
			<< "Failed to load node from config"
			<< ", node id: " << nodeConfig->id
			<< ", node class: " << nodeConfig->className;
		return false;
	}

	return true;
}

bool NodeGraph::allocatePinFromConfig(NodePinConfigPtr pinConfig)
{
	NodePinFactoryPtr factory = getPinFactory(pinConfig->className);

	if (factory)
	{
		NodePinPtr pin = factory->allocatePin();
		if (pin)
		{
			m_Pins.insert({pinConfig->id, pin});

			return true;
		}
		else
		{
			MIKAN_LOG_INFO("NodeGraph::loadPinFromConfig") << "Pin allocation failure";
		}
	}
	else
	{
		MIKAN_LOG_INFO("NodeGraph::loadPinFromConfig") << "Missing factory";
	}

	MIKAN_LOG_INFO("NodeGraph::loadPinFromConfig")
		<< "Failed to load pin from config"
		<< ", parent node id: " << pinConfig->ownerNodeId
		<< ", pin id: " << pinConfig->id
		<< ", pin class: " << pinConfig->className;

	return false;
}

bool NodeGraph::loadPinFromConfig(NodePinConfigPtr pinConfig)
{
	NodePinPtr pin = getPinById(pinConfig->id);
	assert(pin);

	if (!pin->loadFromConfig(shared_from_this(), pinConfig))
	{
		MIKAN_LOG_INFO("NodeGraph::loadPinFromConfig")
			<< "Failed to load pin from config"
			<< ", parent node id: " << pinConfig->ownerNodeId
			<< ", pin id: " << pinConfig->id
			<< ", pin class: " << pinConfig->className;
		return false;
	}

	return true;
}

bool NodeGraph::allocateLinkFromConfig(NodeLinkConfigPtr linkConfig)
{
	NodeLinkPtr link = std::make_shared<NodeLink>();
	link->setOwnerGraph(shared_from_this());
	
	if (linkConfig->id >= 0)
	{
		m_Links.insert({linkConfig->id, link});

		return true;
	}
	else
	{
		MIKAN_LOG_INFO("NodeGraph::allocateLinkFromConfig") << "Config parse failure";
	}

	MIKAN_LOG_INFO("NodeGraph::allocateLinkFromConfig")
		<< "Failed to load link from config"
		<< ", start pin id: " << linkConfig->start_pin_id
		<< ", end pin id: " << linkConfig->end_pin_id
		<< ", link id: " << linkConfig->id;

	return false;
}

bool NodeGraph::loadLinkFromConfig(NodeLinkConfigPtr linkConfig)
{
	NodeLinkPtr link = getLinkById(linkConfig->id);
	assert(link);

	if (!link->loadFromConfig(linkConfig))
	{
		MIKAN_LOG_INFO("NodeGraph::loadLinkFromConfig")
			<< "Failed to load link from config"
			<< ", start pin id: " << linkConfig->start_pin_id
			<< ", end pin id: " << linkConfig->end_pin_id
			<< ", link id: " << linkConfig->id;
		return false;
	}

	return true;
}

void NodeGraph::saveToConfig(NodeGraphConfig& config) const
{
	config.className= getClassName();
	config.nextId= m_nextId;

	// Save all asset references 
	for (auto assetRef : m_assetReferences)
	{
		saveAssetRefToConfig(assetRef, config);
	}

	// Load all properties (depends on asset references)
	for (auto it = m_properties.begin(); it != m_properties.end(); it++)
	{
		saveGraphPropertyToConfig(it->second, config);
	}

	// Load all nodes (depends on properties)
	for (auto it = m_Nodes.begin(); it != m_Nodes.end(); it++)
	{
		saveNodeToConfig(it->second, config);
	}

	// Load all pins (depends on nodes)
	for (auto it = m_Pins.begin(); it != m_Pins.end(); it++)
	{
		savePinToConfig(it->second, config);
	}

	// Load all links (depends on pins)
	for (auto it = m_Links.begin(); it != m_Links.end(); it++)
	{
		saveLinkToConfig(it->second, config);
	}
}

void NodeGraph::saveGraphPropertyToConfig(GraphPropertyConstPtr prop, NodeGraphConfig& graphConfig) const
{
	auto factory= getPropertyFactory(prop->getClassName());
	if (factory)
	{
		auto config= factory->allocatePropertyConfig();
		if (config)
		{
			prop->saveToConfig(config);
			graphConfig.propertyConfigMap.insert({config->id, config});
		}
	}
}

void NodeGraph::saveAssetRefToConfig(AssetReferenceConstPtr assetRef, NodeGraphConfig& graphConfig) const
{
	auto factory = getAssetReferenceFactory(assetRef->getClassName());
	if (factory)
	{
		auto config = factory->allocateAssetReferenceConfig();
		if (config)
		{
			assetRef->saveToConfig(config);
			graphConfig.assetRefConfigs.push_back(config);
		}
	}
}

void NodeGraph::saveNodeToConfig(NodeConstPtr node, NodeGraphConfig& graphConfig) const
{
	auto factory = getNodeFactory(node->getClassName());
	if (factory)
	{
		auto config = factory->allocateNodeConfig();
		if (config)
		{
			node->saveToConfig(config);
			graphConfig.nodeConfigs.push_back(config);
		}
	}
}

void NodeGraph::savePinToConfig(NodePinConstPtr pin, NodeGraphConfig& graphConfig) const
{
	auto factory = getPinFactory(pin->getClassName());
	if (factory)
	{
		auto config = factory->allocatePinConfig();
		if (config)
		{
			pin->saveToConfig(config);
			graphConfig.pinConfigs.push_back(config);
		}
	}
}

void NodeGraph::saveLinkToConfig(NodeLinkConstPtr link, NodeGraphConfig& graphConfig) const
{
	auto config = std::make_shared<NodeLinkConfig>();
	
	link->saveToConfig(config);
	graphConfig.linkConfigs.push_back(config);
}

int NodeGraph::getAssetReferenceIndex(AssetReferencePtr assetRef) const
{
	auto it = std::find(m_assetReferences.begin(), m_assetReferences.end(), assetRef);
	if (it != m_assetReferences.end())
	{
		return it - m_assetReferences.begin();
	}

	return -1;
}

bool NodeGraph::deleteAssetReference(AssetReferencePtr assetRef)
{
	auto it = std::find(m_assetReferences.begin(), m_assetReferences.end(), assetRef);
	if (it != m_assetReferences.end())
	{
		if (OnAssetReferenceDeleted)
			OnAssetReferenceDeleted(assetRef);

		m_assetReferences.erase(it);
		return true;
	}

	return false;
}

void NodeGraph::addProperty(GraphPropertyPtr property)
{
	m_properties.insert({property->getId(), property});

	if (OnPropertyCreated)
		OnPropertyCreated(property->getId());
}

bool NodeGraph::deletePropertyById(t_graph_property_id id)
{
	auto it = m_properties.find(id);
	if (it != m_properties.end())
	{
		t_graph_property_id id = it->first;
		GraphPropertyPtr property = it->second;

		// Signal any listeners that this property is getting deleted first
		if (OnPropertyDeleted)
			OnPropertyDeleted(id);

		// If this property's parent is a container, remove this property from it
		// This will signal the OnPropertyModified event on the parent
		t_graph_property_id parentId= property->getParentId();
		if (parentId != -1)
		{
			auto parentArrayProperty = getTypedPropertyById<GraphArrayProperty>(parentId);
			if (parentArrayProperty)
			{
				parentArrayProperty->removeProperty(property);
			}
		}

		// Remove this property from the graph property table
		// (this should be the last reference to the property)
		m_properties.erase(it);

		return true;
	}

	return false;
}

GraphPropertyPtr NodeGraph::getPropertyById(t_graph_property_id id) const
{
	auto it = m_properties.find(id);
	if (it != m_properties.end())
	{
		return it->second;
	}

	return GraphPropertyPtr();
}

GraphPropertyPtr NodeGraph::getPropertyByName(const std::string& name) const
{
	auto it= std::find_if(
		m_properties.begin(), 
		m_properties.end(), 
		[name](const auto& elem){ return elem.second->getName() == name; });
	if (it != m_properties.end())
	{
		return it->second;
	}

	return GraphPropertyPtr();
}

NodePtr NodeGraph::getNodeById(t_node_id id) const
{
	auto it = m_Nodes.find(id);
	if (it != m_Nodes.end())
	{
		return it->second;
	}

	return NodePtr();
}

NodePtr NodeGraph::getEventNodeByName(const std::string& eventName) const
{
	return getNodeByPredicate(
		[eventName](const auto& elem)
		{ 
			EventNodePtr eventNode= std::dynamic_pointer_cast<EventNode>(elem.second);

			return eventNode && eventNode->getName() == eventName; 
		});
}

NodePinPtr NodeGraph::getPinById(t_node_pin_id id) const
{
	auto it = m_Pins.find(id);
	if (it != m_Pins.end())
	{
		return it->second;
	}

	return NodePinPtr();
}

NodeLinkPtr NodeGraph::getLinkById(t_node_link_id id) const
{
	auto it = m_Links.find(id);
	if (it != m_Links.end())
	{
		return it->second;
	}

	return NodeLinkPtr();
}

NodePtr NodeGraph::createNode(NodeFactoryPtr nodeFactory, const NodeEditorState& nodeEditorState)
{
	NodePtr newNode = nodeFactory->createNode(nodeEditorState);
	if (newNode)
	{
		addNode(newNode);
	}

	return newNode;
}

void NodeGraph::addNode(NodePtr node)
{
	// Add the node to the node map
	m_Nodes.insert({node->getId(), node});

	if (OnNodeCreated)
	{
		OnNodeCreated(node->getId());
	}
}

bool NodeGraph::deleteNodeById(t_node_id id)
{
	auto it = m_Nodes.find(id);
	if (it != m_Nodes.end())
	{
		NodePtr node= it->second;

		// Delete all pins and associated links from this node
		node->disconnectAllPins();
		
		if (OnNodeDeleted)
			OnNodeDeleted(id);

		// Erase the Node
		m_Nodes.erase(it);

		return true;
	}

	return false;
}

void NodeGraph::addPin(NodePinPtr newPin)
{
	if (newPin)
	{
		m_Pins.insert({newPin->getId(), newPin});

		if (OnPinCreated)
		{
			OnPinCreated(newPin->getId());
		}
	}
}

bool NodeGraph::deletePinById(t_node_pin_id id)
{
	auto it = m_Pins.find(id);
	if (it != m_Pins.end())
	{
		NodePinPtr pin = it->second;

		// Delete all link associated with this pin
		// (notify editor dependent links are going away first)
		auto& links= pin->getConnectedLinks();
		while (links.size() > 0)
		{
			const t_node_link_id linkId= links[0]->getId();

			deleteLinkById(linkId);
		}

		// Let the editor know the pin is about to be deleted
		if (OnPinDeleted)
			OnPinDeleted(id);

		// Remove the Pin from the owning Node
		NodePtr ownerNode= pin->getOwnerNode();
		if (ownerNode)
		{
			ownerNode->disconnectPin(pin);
		}

		// Erase the Pin
		m_Pins.erase(it);

		return true;
	}

	return false;
}

NodeLinkPtr NodeGraph::createLink(t_node_pin_id startPinId, t_node_pin_id endPinId)
{
	NodePinPtr startPin = getPinById(startPinId);
	assert(startPin);
	NodePinPtr endPin = getPinById(endPinId);
	assert(endPin);

	// Create a new link and assign the pins to each end
	NodeGraphPtr ownerGraph = shared_from_this();
	NodeLinkPtr link = std::make_shared<NodeLink>();
	link->setOwnerGraph(ownerGraph);
	link->setId(allocateId());
	link->setStartPin(startPin);
	link->setEndPin(endPin);

	// Connect the start and end pin to the link
	// Let each pin decide how it wants to deal with the new link
	// (i.e. if there are existing links, should they be disconnected?)
	startPin->connectLink(link);
	endPin->connectLink(link);

	// Register the link to the graph
	m_Links.insert({link->getId(), link});

	// Let the editor know the link was created
	if (OnLinkCreated)
		OnLinkCreated(link->getId());

	return link;
}

bool NodeGraph::deleteLinkById(t_node_link_id id)
{
	auto it = m_Links.find(id);
	if (it != m_Links.end())
	{
		NodeLinkPtr link= it->second;

		// Let the editor know the link is about to be deleted
		if (OnLinkDeleted)
			OnLinkDeleted(id);

		// Remove link attachments from start and end pins
		NodePinPtr startPin= link->getStartPin();
		assert(startPin);
		NodePinPtr endPin= link->getEndPin();
		assert(endPin);

		startPin->disconnectLink(link);
		endPin->disconnectLink(link);

		// Free the link
		m_Links.erase(it);

		return true;
	}

	return false;
}

std::vector<NodeFactoryPtr> NodeGraph::editorGetValidNodeFactories(const NodeEditorState& editorState) const
{
	std::vector<NodeFactoryPtr> validFactories;

	NodePinPtr sourcePin;
	if (editorState.startedLinkPinId != -1)
	{
		sourcePin= getPinById(editorState.startedLinkPinId);
	}

	for (auto it = m_nodeFactories.begin(); it != m_nodeFactories.end(); ++it)
	{
		NodeFactoryPtr factory = it->second;
		bool bIsValidFactory= factory->editorCanCreate();

		if (bIsValidFactory && sourcePin)
		{
			NodeConstPtr nodeDefaultObject = factory->getNodeDefaultObject();

			if (sourcePin->getDirection() == eNodePinDirection::INPUT)
			{
				for (NodePinPtr targetPin : nodeDefaultObject->getOutputPins())
				{
					if (targetPin->canPinsBeConnected(sourcePin))
					{
						bIsValidFactory = true;
						break;
					}
				}
			}
			else if (sourcePin->getDirection() == eNodePinDirection::OUTPUT)
			{
				for (NodePinPtr targetPin : nodeDefaultObject->getInputPins())
				{
					if (targetPin->canPinsBeConnected(sourcePin))
					{
						bIsValidFactory = true;
						break;
					}
				}
			}
		}

		if (bIsValidFactory)
		{
			validFactories.push_back(factory);
		}
	}

	return validFactories;
}

void NodeGraph::editorRender(const NodeEditorState& editorState)
{
	// Nodes rendering
	for (auto it= m_Nodes.begin(); it != m_Nodes.end(); ++it)
	{
		NodePtr node= it->second;

		if (ImNodes::IsNodeSelected(node->getId()))
		{
			ImNodes::PushStyleVar(ImNodesStyleVar_NodeBorderThickness, 2.6f);
			ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(220, 140, 0, 255));
		}
		else
		{
			ImNodes::PushStyleVar(ImNodesStyleVar_NodeBorderThickness, 2.0f);
			ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(24, 24, 24, 255));
		}

		node->editorRenderNode(editorState);

		const ImVec2 nodePos = ImNodes::GetNodeScreenSpacePos(node->getId());
		node->setNodePos({nodePos.x, nodePos.y});

		ImNodes::PopColorStyle();
		ImNodes::PopStyleVar();
	}

	// Links Rendering
	for (auto it= m_Links.begin(); it != m_Links.end(); ++it)
	{
		NodeLinkPtr link = it->second;

		link->editorRender(editorState);
	}
}

int NodeGraph::allocateId()
{
	int newId = m_nextId;
	m_nextId++;
	return newId;
}

// -- NodeGraphFactory -----
std::map<std::string, NodeGraphFactoryPtr> NodeGraphFactory::s_factoryMap;

NodeGraphPtr NodeGraphFactory::loadNodeGraph(
	IGlWindow* ownerWindow,
	const std::filesystem::path& path)
{
	// Load the node graph config from the file path
	NodeGraphConfig config;
	if (!config.load(path))
	{
		MIKAN_LOG_ERROR("NodeGraphFactory::loadNodeGraph") << "Failed to load NodeGraph Config: " << path;
		return NodeGraphPtr();
	}

	// Find the appropriate factory based on the node class name
	const std::string& nodeGraphClassName = config.className;
	auto it = s_factoryMap.find(nodeGraphClassName);
	if (it == s_factoryMap.end())
	{
		MIKAN_LOG_ERROR("NodeGraphFactory::loadNodeGraph") << "Unknown node graph class name: " << nodeGraphClassName;
		return NodeGraphPtr();
	}

	// Allocate the node graph using the factory and config
	NodeGraphPtr nodeGraph = it->second->allocateNodeGraph();
	if (!nodeGraph)
	{
		MIKAN_LOG_ERROR("NodeGraphFactory::loadNodeGraph") << "Failed to allocate node graph class: " << nodeGraphClassName;
		return NodeGraphPtr();
	}

	// Assign owner window before any graph loading operations allocate GL resources (shaders, textures, etc)
	// which are dependent on the owning window being assigned
	nodeGraph->setOwnerWindow(ownerWindow);

	// Now that we have allocate a node graph using the class name
	// we can actually create the graph object configs using the factories from the graph
	if (!config.postReadFromJSON(nodeGraph))
	{
		MIKAN_LOG_ERROR("NodeGraphFactory::loadNodeGraph") << "Failed to create all graph object configs in graph class: " << nodeGraphClassName;
		return NodeGraphPtr();
	}

	// Init node graph from the parsed config
	if (!nodeGraph->loadFromConfig(config))
	{
		MIKAN_LOG_ERROR("NodeGraphFactory::loadNodeGraph") << "Failed to init all graph objects in graph class: " << nodeGraphClassName;
		return NodeGraphPtr();
	}

	return nodeGraph;
}

void NodeGraphFactory::saveNodeGraph(const std::filesystem::path& path, NodeGraphConstPtr nodeGraph)
{
	NodeGraphConfig config;
	nodeGraph->saveToConfig(config);

	config.save(path);
}

NodeGraphPtr NodeGraphFactory::allocateNodeGraph() const
{
	return std::make_shared<NodeGraph>();
}

NodeGraphPtr NodeGraphFactory::initialCreateNodeGraph(IGlWindow* ownerWindow) const
{
	// Derived node graph types override this method to create properties and nodes
	// on initial creation of the graph.
	NodeGraphPtr nodeGraph= allocateNodeGraph();	

	// Assign owner window before any graph loading operations allocate GL resources (shaders, textures, etc)
	// which are dependent on the owning window being assigned
	nodeGraph->setOwnerWindow(ownerWindow);

	// Create any graph dependent resources (shaders, meshes, etc)
	if (!nodeGraph->createResources())
	{
		MIKAN_LOG_ERROR("NodeGraphFactory::initialCreateNodeGraph") 
			<< "Failed to create graph resources objects in graph class: " << nodeGraph->getClassName();
		return NodeGraphPtr();
	}

	return nodeGraph;
}