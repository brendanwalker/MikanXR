#pragma once

#include "Node.h"
#include "RendererFwd.h"
#include "FrameCompositorConstants.h"

class DrawLayerNodeConfig : public NodeConfig
{
public:
	DrawLayerNodeConfig() : NodeConfig() {}
	DrawLayerNodeConfig(const std::string& nodeName) : NodeConfig(nodeName) {}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eCompositorBlendMode blendMode = eCompositorBlendMode::blendOn;
	bool bVerticalFlip= false;
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
	virtual std::string editorGetTitle() const override { return "Draw Layer"; }

	void setMaterialPin(PropertyPinPtr inPin);
	void onGraphLoaded(bool success);
	virtual void onLinkConnected(NodeLinkPtr link, NodePinPtr pin) override;
	virtual void onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin) override;
	void rebuildInputPins();

	void setMaterial(GlMaterialPtr inMaterial);

	struct QuadVertex
	{
		glm::vec2 aPos;
		glm::vec2 aTexCoords;
	};
	static const struct GlVertexDefinition& getVertexDefinition();

protected:
	std::vector<NodePinPtr> m_dynamicMaterialPins;
	GlTriangulatedMeshPtr m_layerVFlippedQuad;
	GlTriangulatedMeshPtr m_layerMesh;
	GlMaterialPtr m_material;
	PropertyPinPtr m_materialPin;
	eCompositorBlendMode m_blendMode = eCompositorBlendMode::blendOn;
	bool m_bVerticalFlip= false;

	friend class DrawLayerNodeFactory;
};

class DrawLayerNodeFactory : public TypedNodeFactory<DrawLayerNode, DrawLayerNodeConfig>
{
public:
	DrawLayerNodeFactory() = default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};