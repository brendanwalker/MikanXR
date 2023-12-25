#pragma once

#include "Node.h"
#include "RendererFwd.h"

class DrawTriMeshNode : public Node
{
public:
	DrawTriMeshNode() = default;
	virtual ~DrawTriMeshNode();

	inline static const std::string k_nodeClassName = "DrawTriMeshNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	virtual bool evaluateNode(NodeEvaluator& evaluator);
	virtual FlowPinPtr getOutputFlowPin() const;
	virtual bool hasAnyFlowPins() const override { return true; }

	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual std::string editorGetTitle() const override;

	void setMaterialPin(MaterialPinPtr inPin);
	void setModelPin(ModelPinPtr inPin);
	void onGraphLoaded(bool success);
	void onMaterialLinkConnected(t_node_link_id id);
	void onMaterialLinkDisconnected(t_node_link_id id);
	void onModelLinkConnected(t_node_link_id id);
	void onModelLinkDisconnected(t_node_link_id id);
	void rebuildInputPins();

	void setModel(GlRenderModelResourcePtr inModel);
	void setMaterial(GlMaterialPtr inMaterial);

protected:
	GlRenderModelResourcePtr m_model;
	GlMaterialPtr m_material;
	MaterialPinPtr m_materialPin;
	ModelPinPtr m_modelPin;

	friend class DrawTriMeshNodeFactory;
};

class DrawTriMeshNodeFactory : public TypedNodeFactory<DrawTriMeshNode, NodeConfig>
{
public:
	DrawTriMeshNodeFactory() = default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};