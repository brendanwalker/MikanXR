#pragma once

#include "ComponentFwd.h"
#include "CompositorConstants.h"
#include "Node.h"
#include "MikanRendererFwd.h"

class DrawShapesNodeConfig : public NodeConfig
{
public:
	DrawShapesNodeConfig()
		: NodeConfig()
	{
	}
	DrawShapesNodeConfig(const std::string& nodeName)
		: NodeConfig(nodeName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eCompositorBlendMode blendMode= eCompositorBlendMode::blendOn;
	bool bDepthTest= false;
};

class DrawShapesNode : public Node
{
public:
	DrawShapesNode()= default;
	virtual ~DrawShapesNode()= default;

	inline static const std::string k_nodeClassName= "DrawShapesNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual FlowPinPtr getOutputFlowPin() const override;
	virtual bool hasAnyFlowPins() const override { return true; }

	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	void evaluateQuadShapes(CameraComponentPtr cameraComponent, const glm::mat4& vpMatrix);
	void evaluateBoxShapes(CameraComponentPtr cameraComponent, const glm::mat4& vpMatrix);
	void evaluateModelShapes(CameraComponentPtr cameraComponent, const glm::mat4& vpMatrix);

	void onGraphLoaded(bool success);
	virtual void onLinkConnected(NodeLinkPtr link, NodePinPtr pin) override;
	virtual void onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin) override;

	void setShapesPin(ArrayPinPtr inPin);

	void drawShapeRenderable(
		IMkSceneRenderableConstPtr renderable,
		const glm::mat4& vpMatrix,
		IMkTexturePtr colorTexture);

	virtual std::string editorGetTitle() const override { return "Draw Shapes"; }

protected:
	ArrayPinPtr m_shapesPin;

	eCompositorBlendMode m_blendMode= eCompositorBlendMode::blendOn;
	bool m_bDepthTest= false;

	friend class DrawShapesNodeFactory;
};

class DrawShapesNodeFactory : public TypedNodeFactory<DrawShapesNode, DrawShapesNodeConfig>
{
public:
	DrawShapesNodeFactory()= default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};
