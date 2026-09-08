#pragma once

#include "ComponentFwd.h"
#include "LocText.h"
#include "Node.h"
#include "MkRendererFwd.h"

#include <array>
#include <map>
#include <string>
#include <vector>

class DrawShapeMeshNodeConfig : public NodeConfig
{
public:
	DrawShapeMeshNodeConfig()
		: NodeConfig()
	{
	}
	DrawShapeMeshNodeConfig(const std::string& nodeName)
		: NodeConfig(nodeName)
	{
	}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::map<std::string, float> m_floatDefaults;
	std::map<std::string, std::array<float, 2>> m_float2Defaults;
	std::map<std::string, std::array<float, 3>> m_float3Defaults;
	std::map<std::string, std::array<float, 4>> m_float4Defaults;
};

class DrawShapeMeshNode : public Node
{
public:
	DrawShapeMeshNode()= default;
	virtual ~DrawShapeMeshNode();

	inline static const std::string k_nodeClassName= "DrawShapeMeshNode";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	virtual bool evaluateNode(NodeEvaluator& evaluator) override;
	virtual FlowPinPtr getOutputFlowPin() const override;
	virtual bool hasAnyFlowPins() const override { return true; }

	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;
	// The material pin only takes a shape material
	virtual bool editorCanAcceptProperty(NodePinPtr pin, GraphPropertyPtr property) const override;

protected:
	virtual std::string editorGetTitle() const override { return locText("nodes.drawShapeMeshTitle"); }
	virtual const char* editorGetHeaderIcon() const override;

	void onGraphLoaded(bool success);
	// A reload of the connected material recompiled its program
	void onGraphPropertyModified(t_graph_property_id id);
	virtual void onLinkConnected(NodeLinkPtr link, NodePinPtr pin) override;
	virtual void onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin) override;
	// Re-derives the dynamic pins from the material's uniforms, keeping any
	// existing pin whose name and type still match so its links survive
	void rebuildInputPins();
	void captureDynamicPinDefaultValues();
	void applyDynamicPinDefaultValues();

	void setMaterialPin(PropertyPinPtr inPin);
	void setMaterial(MkMaterialConstPtr inMaterial);

	// Draws the shape's meshes with the material. A mesh whose vertex layout the
	// material's program cannot read is skipped and reported once per evaluation,
	// which fails the evaluation.
	bool drawShapeRenderables(NodeEvaluator& evaluator, const std::vector<IMkSceneRenderableConstPtr>& renderables,
							  const glm::mat4& vpMatrix);
	void drawMesh(IMkMeshConstPtr mesh, const glm::mat4& mvpMatrix);

protected:
	PropertyPinPtr m_materialPin;
	MkMaterialConstPtr m_material;
	MkMaterialInstancePtr m_materialInstance;
	std::map<std::string, float> m_floatDefaults;
	std::map<std::string, std::array<float, 2>> m_float2Defaults;
	std::map<std::string, std::array<float, 3>> m_float3Defaults;
	std::map<std::string, std::array<float, 4>> m_float4Defaults;

	friend class DrawShapeMeshNodeFactory;
};

class DrawShapeMeshNodeFactory : public TypedNodeFactory<DrawShapeMeshNode, DrawShapeMeshNodeConfig>
{
public:
	DrawShapeMeshNodeFactory()= default;

	virtual NodePtr createNode(const class NodeEditorState& editorState) const override;
};
