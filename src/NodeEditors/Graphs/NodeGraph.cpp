#include "NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Logger.h"
#include "Nodes/Node.h"
#include "Nodes/EventNode.h"
#include "Pins/FloatPin.h"
#include "Pins/FlowPin.h"
#include "Pins/IntPin.h"
#include "Pins/NodeLink.h"
#include "Pins/NodePin.h"
#include "Properties/GraphArrayProperty.h"
#include "Properties/GraphAssetListProperty.h"
#include "Properties/GraphVariableList.h"
#include "NodeEditorState.h"

#include "imnodes.h"

#include <filesystem>

// -- NodeGraphFactory -----
std::map<std::string, NodeGraphFactoryPtr> NodeGraphFactory::s_factoryMap;

NodeGraphPtr NodeGraphFactory::loadNodeGraph(const std::filesystem::path& path)
{
	// Load the node graph config from the file path
	NodeGraphConfig config;
	if (!config.load(path))
	{
		MIKAN_LOG_INFO("NodeGraphFactory::loadNodeGraph") << "Failed to load NodeGraph Config: " << path;
		return NodeGraphPtr();
	}

	// Find the appropriate factory based on the node class name
	const std::string& nodeGraphClassName= config.className;
	auto it= s_factoryMap.find(nodeGraphClassName);
	if (it == s_factoryMap.end())
	{
		MIKAN_LOG_INFO("NodeGraphFactory::loadNodeGraph") << "Unknown node graph class name: " << nodeGraphClassName;
		return NodeGraphPtr();
	}

	// Allocate the node graph using the factory and config
	NodeGraphPtr nodeGraph= it->second->allocateNodeGraph();
	if (!nodeGraph)
	{
		MIKAN_LOG_INFO("NodeGraphFactory::loadNodeGraph") << "Failed to allocate node graph class: " << nodeGraphClassName;
		return NodeGraphPtr();
	}

	// Init node graph from the parse config
	if (!nodeGraph->loadFromConfig(config))
	{
		MIKAN_LOG_INFO("NodeGraphFactory::loadNodeGraph") << "Failed to init node graph class: " << nodeGraphClassName;
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

	writeStdConfigVector(pt, "properties", propertyConfigs);
	writeStdConfigVector(pt, "nodes", nodeConfigs);
	writeStdConfigVector(pt, "pins", nodePinConfigs);
	writeStdConfigVector(pt, "links", nodeLinkConfigs);

	return pt;
}

void NodeGraphConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	className = pt.get_or<std::string>("class_name", "NodeGraph");
	nextId = pt.get_or<int>("next_id", -1);

	readStdConfigVector(pt, "properties", propertyConfigs);
	readStdConfigVector(pt, "nodes", nodeConfigs);
	readStdConfigVector(pt, "pins", nodePinConfigs);
	readStdConfigVector(pt, "links", nodeLinkConfigs);
}

// -- NodeGraph -----
NodeGraph::NodeGraph()
{
	// Add pin types nodes in this graph can use
	addPinFactory<NodePin>();
	addPinFactory<FlowPin>();
	addPinFactory<FloatPin>();
	addPinFactory<Float2Pin>();
	addPinFactory<Float3Pin>();
	addPinFactory<Float4Pin>();
	addPinFactory<IntPin>();
	addPinFactory<Int2Pin>();
	addPinFactory<Int3Pin>();
	addPinFactory<Int4Pin>();

	// Add property types this graph can use
	addPropertyFactory< TypedGraphPropertyFactory<GraphArrayProperty> >();
	addPropertyFactory< TypedGraphPropertyFactory<GraphAssetListProperty> >();
	addPropertyFactory< TypedGraphPropertyFactory<GraphVariableList> >();

	// Add graph properties
	addTypedProperty<GraphAssetListProperty>("assetReferences");
}

bool NodeGraph::loadFromConfig(const NodeGraphConfig& config)
{
	m_nextId= config.nextId;

	for (auto propConfig : config.propertyConfigs)
	{

	}

	return true;
}

void NodeGraph::saveToConfig(NodeGraphConfig& config) const
{
	config.className= typeid(*this).name();
}

void NodeGraph::update(NodeEvaluator& evaluator)
{
	m_timeInSeconds+= evaluator.getDeltaSeconds();

	NodePtr onTickNode = getEventNodeByName("OnTick");
	if (onTickNode)
	{
		evaluator.evaluateFlowPinChain(onTickNode);
		if (evaluator.getLastErrorCode() != eNodeEvaluationErrorCode::NONE)
		{
			MIKAN_LOG_ERROR("NodeGraph::update - Error: ") << evaluator.getLastErrorMessage();
		}
	}
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

NodePinPtr NodeGraph::getNodePinById(t_node_pin_id id) const
{
	auto it = m_Pins.find(id);
	if (it != m_Pins.end())
	{
		return it->second;
	}

	return NodePinPtr();
}

NodeLinkPtr NodeGraph::getNodeLinkById(t_node_link_id id) const
{
	auto it = m_Links.find(id);
	if (it != m_Links.end())
	{
		return it->second;
	}

	return NodeLinkPtr();
}

NodePtr NodeGraph::createNode(NodeFactoryPtr nodeFactory, const NodeEditorState* nodeEditorState)
{
	NodePtr newNode = nodeFactory->createNode(nodeEditorState);

	if (newNode)
	{
		// Add the node to the node map
		m_Nodes.insert({newNode->getId(), newNode});

		if (OnNodeCreated)
		{
			OnNodeCreated(newNode->getId());
		}
	}

	return newNode;
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

NodePinPtr NodeGraph::createPin(
	NodePinFactoryPtr pinFactory,
	NodePtr ownerNode,
	const std::string& name, 
	eNodePinDirection direction)
{
	NodePinPtr newPin = pinFactory->createPin(ownerNode, name, direction);
	addPin(newPin);

	return newPin;
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
	NodePinPtr startPin = getNodePinById(startPinId);
	assert(startPin);
	NodePinPtr endPin = getNodePinById(endPinId);
	assert(endPin);

	// Create a new link and assign the pins to each end
	NodeGraphPtr ownerGraph = shared_from_this();
	NodeLinkPtr link = std::make_shared<NodeLink>(ownerGraph);
	link->setStartPin(startPin);
	link->setEndPin(endPin);

	// Connect the start and end pin to the link
	// Let each pin decide how it wants to deal with the new link
	// (i.e. if there are existing links, should they be disconnected?)
	startPin->connectLink(link);
	endPin->connectLink(link);

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

	if (editorState.startedLinkPinId != -1)
	{
		NodePinPtr sourcePin= getNodePinById(editorState.startedLinkPinId);

		for (auto it = m_nodeFactories.begin(); it != m_nodeFactories.end(); ++it)
		{
			NodeFactoryPtr factory = it->second;
			NodeConstPtr nodeDefaultObject= factory->getNodeDefaultObject();
			bool bIsValidFactory= false;

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

			if (bIsValidFactory)
			{
				validFactories.push_back(factory);
			}
		}
	}
	else
	{
		// Return all available factories
		for (auto it = m_nodeFactories.begin(); it != m_nodeFactories.end(); ++it)
		{
			NodeFactoryPtr factory = it->second;
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