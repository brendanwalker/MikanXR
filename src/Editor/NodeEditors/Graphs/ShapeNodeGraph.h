#pragma once

#include "NodeGraph.h"
#include "ComponentFwd.h"

#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class ShapeNodeGraph : public NodeGraph
{
public:
	ShapeNodeGraph();

	static const std::string k_renderShapeEventName;

	virtual std::string getClassName() const override { return "ShapeNodeGraph"; }
	virtual bool loadFromConfig(const NodeGraphConfig& config) override;

	// Evaluation — sets the transient VP matrix, evaluates the OnRenderShape chain
	bool renderShape(const glm::mat4& vpMatrix, NodeEvaluator& evaluator);

	// Binding
	bool bindToShapeComponent(ShapeComponentPtr shapeComponent);
	void unbindFromShapeComponent(ShapeComponentPtr shapeComponent);
	ShapeComponentPtr getBoundShapeComponent() const;

	// Transient VP matrix — set before evaluation, cleared after
	const glm::mat4& getRenderVPMatrix() const { return m_vpMatrix; }

protected:
	bool bindEventNodes();

	ShapeComponentWeakPtr m_boundShapeComponent;
	glm::mat4 m_vpMatrix= glm::mat4(1.f);
	NodePtr m_renderShapeEventNode;

	friend class ShapeNodeGraphFactory;
};

class ShapeNodeGraphFactory : public TypedNodeGraphFactory<ShapeNodeGraph>
{
public:
	ShapeNodeGraphFactory()= default;

	virtual NodeGraphPtr initialCreateNodeGraph(class IEditorWindow* ownerWindow) const override;
};
