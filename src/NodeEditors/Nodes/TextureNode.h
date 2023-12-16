#pragma once

#include "Node.h"
#include "RendererFwd.h"

class TextureNode : public Node
{
public:
	TextureNode();
	TextureNode(NodeGraphPtr ownerGraph);
	virtual ~TextureNode();

	inline GraphTexturePropertyPtr getTextureSource() const { return m_sourceProperty; }
	void setTextureSource(GraphTexturePropertyPtr inTextureProperty);

	GlTexturePtr getTextureResource() const;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;

protected:
	virtual void editorRenderPushNodeStyle(const NodeEditorState& editorState) const override;
	virtual std::string editorGetTitle() const override { return "Texture"; }

	void onGraphPropertyChanged(t_graph_property_id id);

protected:
	GraphTexturePropertyPtr m_sourceProperty;
	GraphVariableListPtr m_textureArrayProperty;
};

class TextureNodeFactory : public NodeFactory
{
public:
	TextureNodeFactory() = default;
	TextureNodeFactory(NodeGraphPtr ownerGraph) : NodeFactory(ownerGraph) {}

	virtual NodePtr createNode(const class NodeEditorState* editorState) const override;
};