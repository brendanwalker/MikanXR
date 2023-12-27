#pragma once

#include "Node.h"
#include "RendererFwd.h"

class DrawLayerNode : public Node
{
public:
	DrawLayerNode() = default;
	virtual ~DrawLayerNode();

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

protected:
	std::vector<NodePinPtr> m_dynamicMaterialPins;
	GlRenderModelResourcePtr m_model;
	GlMaterialPtr m_material;
	PropertyPinPtr m_materialPin;

	friend class DrawLayerNodeFactory;
};

class DrawLayerNodeFactory : public TypedNodeFactory<DrawLayerNode, NodeConfig>
{
public:
	DrawLayerNodeFactory() = default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};