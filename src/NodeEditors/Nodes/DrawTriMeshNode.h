#pragma once

#include "Node.h"
#include "RendererFwd.h"

class DrawTriMeshNode : public Node
{
public:
	DrawTriMeshNode();
	DrawTriMeshNode(NodeGraphPtr parentGraph);
	virtual ~DrawTriMeshNode();

	void setTriangulatedMesh(GlTriangulatedMeshPtr inTriMesh);
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
	GlTriangulatedMeshPtr m_triMesh;
	GlMaterialPtr m_material;

	TriMeshArrayPropertyPtr m_triMeshArrayProperty;
};

class DrawTriMeshNodeFactory : public NodeFactory
{
public:
	DrawTriMeshNodeFactory() = default;
	DrawTriMeshNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};