#pragma once

#include "Node.h"
#include "MikanRendererFwd.h"
#include "FrameCompositorConstants.h"

#include <array>
#include <map>
#include <string>

/// The ID of a stencil
typedef int32_t MikanStencilID;

class DrawLayerNodeConfig : public NodeConfig
{
public:
	DrawLayerNodeConfig() : NodeConfig() {}
	DrawLayerNodeConfig(const std::string& nodeName) : NodeConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eCompositorBlendMode blendMode = eCompositorBlendMode::blendOn;
	eCompositorStencilMode stencilMode = eCompositorStencilMode::insideStencil;
	bool bVerticalFlip= false;
	bool bInvertWhenCameraInside= false;
	std::map<std::string, float> m_floatDefaults;
	std::map<std::string, std::array<float, 2> > m_float2Defaults;
	std::map<std::string, std::array<float, 3> > m_float3Defaults;
	std::map<std::string, std::array<float, 4> > m_float4Defaults;
};

class DrawLayerNode : public Node
{
public:
	DrawLayerNode() = default;
	virtual ~DrawLayerNode();

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig);
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const;

	inline static const std::string k_nodeClassName = "DrawLayerNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	virtual bool evaluateNode(NodeEvaluator& evaluator);
	virtual FlowPinPtr getOutputFlowPin() const;
	virtual bool hasAnyFlowPins() const override { return true; }

	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	void evaluateQuadStencils(GlState& glParentState);
	void evaluateBoxStencils(GlState& glParentState);
	void evaluateModelStencils(GlState& glParentState);

	virtual std::string editorGetTitle() const override { return "Draw Layer"; }

	void onGraphLoaded(bool success);
	virtual void onLinkConnected(NodeLinkPtr link, NodePinPtr pin) override;
	virtual void onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin) override;
	void rebuildInputPins();
	void applyDynamicPinDefaultValues();

	void setMaterialPin(PropertyPinPtr inPin);
	void setMaterial(GlMaterialConstPtr inMaterial);

	void setStencilsPin(ArrayPinPtr inPin);
	void rebuildStencilLists();

protected:
	ArrayPinPtr m_stencilsPin;
	std::vector<MikanStencilID> m_quadStencilIds;
	std::vector<MikanStencilID> m_boxStencilIds;
	std::vector<MikanStencilID> m_modelStencilIds;

	PropertyPinPtr m_materialPin;
	GlMaterialConstPtr m_material;
	GlMaterialInstancePtr m_materialInstance;
	std::map<std::string, float> m_floatDefaults;
	std::map<std::string, std::array<float, 2> > m_float2Defaults;
	std::map<std::string, std::array<float, 3> > m_float3Defaults;
	std::map<std::string, std::array<float, 4> > m_float4Defaults;

	eCompositorBlendMode m_blendMode = eCompositorBlendMode::blendOn;
	eCompositorStencilMode m_stencilMode = eCompositorStencilMode::insideStencil;
	bool m_bVerticalFlip= false;
	bool m_bInvertWhenCameraInside= false;

	friend class DrawLayerNodeFactory;
};

class DrawLayerNodeFactory : public TypedNodeFactory<DrawLayerNode, DrawLayerNodeConfig>
{
public:
	DrawLayerNodeFactory() = default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};