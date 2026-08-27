#pragma once

#include "ComponentFwd.h"
#include "LocText.h"
#include "Node.h"
#include "MikanRendererFwd.h"

#include <array>
#include <map>
#include <string>

class ApplyMaterialNodeConfig : public NodeConfig
{
public:
	ApplyMaterialNodeConfig()
		: NodeConfig()
	{
	}
	ApplyMaterialNodeConfig(const std::string& nodeName)
		: NodeConfig(nodeName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::map<std::string, float> m_floatDefaults;
	std::map<std::string, std::array<float, 2>> m_float2Defaults;
	std::map<std::string, std::array<float, 3>> m_float3Defaults;
	std::map<std::string, std::array<float, 4>> m_float4Defaults;
};

// Renders a material to an internal frame buffer and exposes the result as a Texture output pin.
// Has no exec/flow pins - it's a pure "pull" node meant to be spliced between a texture-producing
// node (e.g. ColorTextureSourceNode) and a texture-consuming node (e.g. DrawLayerNode), so that
// post-process effects (grain, color grading, etc.) can be reused as modular building blocks
// instead of being baked into a bespoke DrawLayerNode material.
class ApplyMaterialNode : public Node
{
public:
	ApplyMaterialNode();
	virtual ~ApplyMaterialNode();

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline static const std::string k_nodeClassName= "ApplyMaterialNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;

	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual std::string editorGetTitle() const override { return locText("nodes.applyMaterialTitle"); }
	virtual const char* editorGetHeaderIcon() const override;

	void onGraphLoaded(bool success);
	virtual void onLinkConnected(NodeLinkPtr link, NodePinPtr pin) override;
	virtual void onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin) override;
	void rebuildInputPins();
	void applyDynamicPinDefaultValues();

	void setMaterialPin(PropertyPinPtr inPin);
	void setMaterial(MkMaterialConstPtr inMaterial);
	void setOutputTexturePin(TexturePinPtr outPin);

protected:
	PropertyPinPtr m_materialPin;
	MkMaterialConstPtr m_material;
	MkMaterialInstancePtr m_materialInstance;
	std::map<std::string, float> m_floatDefaults;
	std::map<std::string, std::array<float, 2>> m_float2Defaults;
	std::map<std::string, std::array<float, 3>> m_float3Defaults;
	std::map<std::string, std::array<float, 4>> m_float4Defaults;

	TexturePinPtr m_outTexturePin;
	IMkFrameBufferPtr m_outputFrameBuffer;

	friend class ApplyMaterialNodeFactory;
};

class ApplyMaterialNodeFactory : public TypedNodeFactory<ApplyMaterialNode, ApplyMaterialNodeConfig>
{
public:
	ApplyMaterialNodeFactory()= default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};
