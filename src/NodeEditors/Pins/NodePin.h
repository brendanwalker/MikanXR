#pragma once

#include "CommonConfig.h"
#include "NodeFwd.h"
#include "NodePinConstants.h"
#include "MulticastDelegate.h"

#include "imnodes.h"

#include <string>
#include <vector>

class NodePinConfig : public CommonConfig
{
public:
	NodePinConfig() : CommonConfig() {}
	NodePinConfig(const std::string& nodeName) : CommonConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string className;
	std::string pinName;
	t_node_pin_id id;
	eNodePinDirection direction;
	t_node_id ownerNodeId;
	std::vector<t_node_link_id> connectedLinkIds;
};

class NodePin
{
public:
	NodePin();
	NodePin(NodePtr ownerNode);

	virtual bool loadFromConfig(const class NodePinConfig& config);
	virtual void saveToConfig(class NodePinConfig& config) const;

	inline t_node_pin_id getId() const { return m_id; }
	inline eNodePinDirection getDirection() const { return m_direction; }
	inline const std::string& getName() const { return m_name; }
	inline NodePtr getOwnerNode() const { return m_ownerNode; }
	inline const std::vector<NodeLinkPtr>& getConnectedLinks() const { return m_connectedLinks; }

	inline void setName(const std::string& name) { m_name= name; }
	inline void setDirection(eNodePinDirection direction) { m_direction= direction; }

	virtual bool connectLink(NodeLinkPtr linkPtr);
	MulticastDelegate<void(t_node_link_id id)> OnLinkConnected;
	virtual bool disconnectLink(NodeLinkPtr linkPtr);
	MulticastDelegate<void(t_node_link_id id)> OnLinkDisconnected;

	virtual size_t getDataSize() const { return 0; }
	virtual bool canPinsBeConnected(NodePinPtr otherPinPtr) const;

	NodePinPtr getConnectedSourcePin() const;
	virtual void copyValueFromSourcePin() {}

	virtual float editorComputeInputWidth() const;
	virtual float editorComputeNodeAlpha(const NodeEditorState& editorState) const;
	virtual void editorRenderInputPin(const NodeEditorState& editorState);
	virtual void editorRenderInputTextEntry(const NodeEditorState& editorState) {}
	virtual void editorRenderOutputPin(const NodeEditorState& editorState, float prefixWidth= 0.f);
	virtual ImNodesPinShape editorRenderBeginPin(float alpha);
	virtual void editorRenderEndPin();
	virtual void editorRenderBeginLink(float alpha);
	virtual void editorRenderEndLink();
	virtual void editorRenderContextMenu(const NodeEditorState& editorState) {}
	virtual ImU32 editorGetLinkStyleColor() const;

protected:
	t_node_pin_id m_id;
	eNodePinDirection m_direction;
	std::string m_name;
	NodePtr m_ownerNode;
	std::vector<NodeLinkPtr> m_connectedLinks;
};

class NodePinFactory
{
public:
	NodePinFactory() = default;
	NodePinFactory(NodeGraphPtr ownerGraph) : m_ownerGraph(ownerGraph) {}

	inline NodeGraphPtr getOwner() const { return m_ownerGraph; }
	inline std::string getPinClassName() const { return typeid(m_defaultPinObject.get()).name(); }

	virtual NodePinPtr createPin(NodePtr ownerNode, const std::string& name, eNodePinDirection direction) const= 0;

	virtual NodePinConfigPtr createPinConfig() const
	{
		return std::make_shared<NodePinConfig>();
	}

	template <class t_factory_class>
	static NodePinFactoryPtr createFactory(NodeGraphPtr ownerGraph)
	{
		auto factory= std::make_shared<t_factory_class>(ownerGraph);

		// Create a single "node pin object" for the factory.
		// This is used to ask questions about a pin without having to create one first.
		// We have to do this work outside of the NodePinFactory constructor,
		// because virtual functions aren't safe to call in constructor.
		factory->m_defaultPinObject = factory->createPin(nullptr, "", eNodePinDirection::INVALID);

		// Create a node factory instance
		return factory;
	}

protected:
	NodeGraphPtr m_ownerGraph;
	NodePinPtr m_defaultPinObject;
};

template <class t_pin_class>
class TypedNodePinFactory : public NodePinFactory
{
public:
	TypedNodePinFactory() = default;
	TypedNodePinFactory(NodeGraphPtr ownerGraph) : NodePinFactory(ownerGraph) {}

	virtual NodePinPtr createPin(
		NodePtr ownerNode,
		const std::string& name,
		eNodePinDirection direction) const override
	{
		return ownerNode->addPin<t_pin_class>(name, direction);
	}
};