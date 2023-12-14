#pragma once

#include "Node.h"
#include "RendererFwd.h"

class DrawTriMeshNode : public Node
{
public:
	DrawTriMeshNode();
	DrawTriMeshNode(NodeGraphPtr parentGraph);
	virtual ~DrawTriMeshNode();

	void setModel(GlRenderModelResourcePtr inModel);
	void setMaterial(GlMaterialPtr inMaterial);

	virtual bool evaluateNode(NodeEvaluator& evaluator);
	virtual FlowPinPtr getOutputFlowPin() const;
	virtual bool hasAnyFlowPins() const override { return true; }

	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual std::string editorGetTitle() const override;
	void onGraphPropertyChanged(t_graph_property_id id);
	void rebuildInputPins();

protected:
	GlRenderModelResourcePtr m_model;
	GlMaterialPtr m_material;

	GraphVariableListPtr m_modelArrayProperty;
};

class DrawTriMeshNodeFactory : public NodeFactory
{
public:
	DrawTriMeshNodeFactory() = default;
	DrawTriMeshNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};