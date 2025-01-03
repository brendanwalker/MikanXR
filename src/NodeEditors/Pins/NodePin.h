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
	bool hasDefaultValue = false;
	bool isDynamic= false;
};

class NodePin  : public std::enable_shared_from_this<NodePin>
{
public:
	NodePin();

	inline static const std::string k_pinClassName = "NodePin";
	virtual std::string getClassName() const { return k_pinClassName; }

	virtual bool loadFromConfig(NodeGraphPtr ownerGraph, NodePinConfigConstPtr config);
	virtual void saveToConfig(NodePinConfigPtr config) const;

	inline void setId(t_node_pin_id id) { m_id= id; }
	inline t_node_pin_id getId() const { return m_id; }

	inline void setDirection(eNodePinDirection direction) { m_direction= direction; }
	inline eNodePinDirection getDirection() const { return m_direction; }

	inline void setName(const std::string& name) { m_name = name; }
	inline const std::string& getName() const { return m_name; }

	inline void setOwnerNode(NodePtr ownerNode) { m_ownerNode= ownerNode; }
	inline NodePtr getOwnerNode() const { return m_ownerNode; }

	virtual bool connectLink(NodeLinkPtr linkPtr);
	virtual bool disconnectLink(NodeLinkPtr linkPtr);
	inline const std::vector<NodeLinkPtr>& getConnectedLinks() const { return m_connectedLinks; }
	inline bool hasAnyConnectedLinks() const { return m_connectedLinks.size() > 0; }

	// This is used that the pin provides a default value if no value feed into it
	// (Primarily used for input pins to decide if the node can still be evaluated if an input is disconnected)
	inline void setHasDefaultValue(bool bOptional) { m_bHasDefaultValue = bOptional; }
	inline bool getHasDefaultValue() const { return m_bHasDefaultValue; }

	// This is used that the pin that is dynamically created due to another pin
	// (Like a material input pin that creates other parameter pins)
	inline void setIsDynamicPin(bool bIsDynamic) { m_bIsDynamic = bIsDynamic; }
	inline bool getIsDynamicPin() const { return m_bIsDynamic; }

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

	void editorSetShowPinName(bool bFlag) { m_bEditorShowPinName= bFlag; }
	bool editorShowPinName() const { return m_bEditorShowPinName; }

protected:
	t_node_pin_id m_id;
	eNodePinDirection m_direction;
	std::string m_name;
	NodePtr m_ownerNode;
	std::vector<NodeLinkPtr> m_connectedLinks;
	bool m_bHasDefaultValue = false;
	bool m_bIsDynamic= false;

	// Editor Flags
	bool m_bEditorShowPinName= true;
};

class NodePinFactory
{
public:
	NodePinFactory() = default;

	inline std::string getPinClassName() const { return m_defaultPinObject->getClassName(); }

	virtual NodePinPtr allocatePin() const= 0;

	virtual NodePinConfigPtr allocatePinConfig() const
	{
		return std::make_shared<NodePinConfig>();
	}

	template <class t_factory_class>
	static NodePinFactoryPtr createFactory()
	{
		auto factory= std::make_shared<t_factory_class>();

		// Create a single "node pin object" for the factory.
		// This is used to ask questions about a pin without having to create one first.
		// We have to do this work outside of the NodePinFactory constructor,
		// because virtual functions aren't safe to call in constructor.
		factory->m_defaultPinObject = factory->allocatePin();

		// Create a node factory instance
		return factory;
	}

protected:
	NodePinPtr m_defaultPinObject;
};

template <class t_pin_class, class t_pin_config_class>
class TypedNodePinFactory : public NodePinFactory
{
public:
	TypedNodePinFactory() = default;

	virtual NodePinConfigPtr allocatePinConfig() const
	{
		return std::make_shared<t_pin_config_class>();
	}

	virtual NodePinPtr allocatePin() const override
	{
		return std::make_shared<t_pin_class>();
	}
};