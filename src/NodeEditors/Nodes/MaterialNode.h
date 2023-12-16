#pragma once

#include "Node.h"
#include "RendererFwd.h"

class MaterialNode : public Node
{
public:
	MaterialNode();
	MaterialNode(NodeGraphPtr ownerGraph);
	virtual ~MaterialNode();

	inline GraphMaterialPropertyPtr getMaterialSource() const { return m_sourceProperty; }
	void setMaterialSource(GraphMaterialPropertyPtr inMaterialProperty);

	GlMaterialPtr getMaterialResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;

protected:
	void updateMaterialPinValue();

	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Texture"; }

	void onGraphPropertyChanged(t_graph_property_id id);

protected:
	GraphMaterialPropertyPtr m_sourceProperty;
	GraphVariableListPtr m_materialArrayProperty;
};

class MaterialNodeFactory : public NodeFactory
{
public:
	MaterialNodeFactory() = default;
	MaterialNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};