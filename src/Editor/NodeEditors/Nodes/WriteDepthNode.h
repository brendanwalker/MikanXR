#pragma once

#include "LocText.h"
#include "Node.h"
#include "MikanRendererFwd.h"

class WriteDepthNodeConfig : public NodeConfig
{
public:
	WriteDepthNodeConfig()
		: NodeConfig()
	{
	}
	WriteDepthNodeConfig(const std::string& nodeName)
		: NodeConfig(nodeName)
	{
	}
};

// Writes a linear depth texture (the output of a DepthMaskNode or a DepthTextureSourceNode) into the
// compositor working framebuffer's depth buffer as hardware depth, so a later depth-tested draw such
// as DrawShapesNode sorts against it. The working depth buffer starts at the far plane each frame and
// the write runs under the default LESS test, so two of these nodes in sequence keep the nearer depth.
class WriteDepthNode : public Node
{
public:
	WriteDepthNode()= default;
	virtual ~WriteDepthNode()= default;

	inline static const std::string k_nodeClassName= "WriteDepthNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual FlowPinPtr getOutputFlowPin() const override;
	virtual bool hasAnyFlowPins() const override { return true; }

protected:
	virtual std::string editorGetTitle() const override { return locText("nodes.writeDepthTitle"); }
	virtual const char* editorGetHeaderIcon() const override;

	TexturePinPtr getDepthTexturePin() const;

protected:
	MkMaterialInstancePtr m_materialInstance;

	friend class WriteDepthNodeFactory;
};

class WriteDepthNodeFactory : public TypedNodeFactory<WriteDepthNode, WriteDepthNodeConfig>
{
public:
	WriteDepthNodeFactory()= default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};
