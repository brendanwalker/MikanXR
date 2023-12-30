#pragma once

#include "NodeGraph.h"
#include "ComponentFwd.h"
#include "RendererFwd.h"

#include <map>
#include <string>

typedef int32_t MikanStencilID;

class CompositorNodeGraph : public NodeGraph
{
public:
	CompositorNodeGraph();

	virtual bool createResources() override;
	virtual void disposeResources() override;

	static const std::string k_compositeFrameEventName;

	virtual std::string getClassName() const override { return "CompositorNodeGraph"; }
	virtual bool loadFromConfig(const NodeGraphConfig& config) override;
	bool compositeFrame(NodeEvaluator& evaluator);

	// Stencil Models
	struct StencilVertex
	{
		glm::vec3 aPos;
	};
	GlRenderModelResourcePtr getOrLoadStencilRenderModel(ModelStencilDefinitionPtr stencilDefinition);
	void flushStencilRenderModel(MikanStencilID stencilId);
	static const GlVertexDefinition& getStencilModelVertexDefinition();
	inline GlProgramPtr getStencilShader() const { return m_stencilShader; }

protected:
	static const GlProgramCode* getStencilShaderCode();
	bool createStencilShader();

	// Stencil System Events
	void onStencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

protected:
	GlProgramPtr m_stencilShader;
	std::map<MikanStencilID, GlRenderModelResourcePtr> m_stencilMeshCache;
	NodePtr m_compositeFrameEventNode;
};

class CompositorNodeGraphFactory : public TypedNodeGraphFactory<CompositorNodeGraph>
{
public:
	CompositorNodeGraphFactory() = default;

	virtual NodeGraphPtr initialCreateNodeGraph(class IGlWindow* ownerWindow) const override;
};
