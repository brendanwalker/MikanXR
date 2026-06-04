#pragma once

#include "ComponentFwd.h"
#include "CompositorConstants.h"
#include "Node.h"
#include "MkRendererFwd.h"

#include <array>
#include <map>
#include <string>

class DrawShapeMeshNodeConfig : public NodeConfig
{
public:
	DrawShapeMeshNodeConfig() : NodeConfig() {}
	DrawShapeMeshNodeConfig(const std::string& nodeName) : NodeConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eCompositorBlendMode blendMode = eCompositorBlendMode::blendOn;
	bool bDepthTest = false;
	std::map<std::string, float> m_floatDefaults;
	std::map<std::string, std::array<float, 2>> m_float2Defaults;
	std::map<std::string, std::array<float, 3>> m_float3Defaults;
	std::map<std::string, std::array<float, 4>> m_float4Defaults;
};

class DrawShapeMeshNode : public Node
{
public:
	DrawShapeMeshNode() = default;
	virtual ~DrawShapeMeshNode() = default;

	inline static const std::string k_nodeClassName = "DrawShapeMeshNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual FlowPinPtr getOutputFlowPin() const override;
	virtual bool hasAnyFlowPins() const override { return true; }

	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual std::string editorGetTitle() const override { return "Draw Shape Mesh"; }

	void onGraphLoaded(bool success);
	virtual void onLinkConnected(NodeLinkPtr link, NodePinPtr pin) override;
	virtual void onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin) override;
	void rebuildInputPins();
	void applyDynamicPinDefaultValues();

	void setMaterialPin(PropertyPinPtr inPin);
	void setMaterial(MkMaterialConstPtr inMaterial);

	void drawShapeRenderable(
		IMkSceneRenderableConstPtr renderable,
		const glm::mat4& vpMatrix);

protected:
	PropertyPinPtr m_materialPin;
	MkMaterialConstPtr m_material;
	MkMaterialInstancePtr m_materialInstance;
	std::map<std::string, float> m_floatDefaults;
	std::map<std::string, std::array<float, 2>> m_float2Defaults;
	std::map<std::string, std::array<float, 3>> m_float3Defaults;
	std::map<std::string, std::array<float, 4>> m_float4Defaults;

	eCompositorBlendMode m_blendMode = eCompositorBlendMode::blendOn;
	bool m_bDepthTest = false;

	friend class DrawShapeMeshNodeFactory;
};

class DrawShapeMeshNodeFactory : public TypedNodeFactory<DrawShapeMeshNode, DrawShapeMeshNodeConfig>
{
public:
	DrawShapeMeshNodeFactory() = default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};
