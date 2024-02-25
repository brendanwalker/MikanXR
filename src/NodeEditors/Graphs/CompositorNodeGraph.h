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
	GlTextureConstPtr getCompositedFrameTexture() const;
	void setExternalCompositedFrameTexture(GlTexturePtr externalTexture);

	// Stencil Models
	GlRenderModelResourcePtr getOrLoadStencilRenderModel(ModelStencilDefinitionPtr stencilDefinition);
	void flushStencilRenderModel(MikanStencilID stencilId);
	inline GlProgramPtr getVertexOnlyStencilShader() const { return m_vertexOnlyStencilShader; }
	inline GlTriangulatedMeshPtr getStencilQuadMesh() const { return m_stencilQuadMesh; }
	inline GlTriangulatedMeshPtr getStencilBoxMesh() const { return m_stencilBoxMesh; }
	inline GlTriangulatedMeshPtr getLayerVFlippedMesh() const { return m_layerVFlippedMesh; }
	inline GlTriangulatedMeshPtr getLayerMesh() const { return m_layerMesh; }

protected:
	// Layer Quad 
	struct QuadVertex
	{
		glm::vec2 aPos;
		glm::vec2 aTexCoords;
	};
	static const GlVertexDefinition& getLayerQuadVertexDefinition();

	bool bindEventNodes();

	bool createStencilShaders();
	bool createLayerQuadMeshes();
	bool createStencilMeshes();
	void updateCompositingFrameBufferSize(NodeEvaluator& evaluator);

	// Stencil System Events
	void onStencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

protected:
	GlFrameBufferPtr m_compositingFrameBuffer;
	GlProgramPtr m_vertexOnlyStencilShader;
	GlTriangulatedMeshPtr m_stencilQuadMesh;
	GlTriangulatedMeshPtr m_stencilBoxMesh;
	GlTriangulatedMeshPtr m_layerVFlippedMesh;
	GlTriangulatedMeshPtr m_layerMesh;
	std::map<MikanStencilID, GlRenderModelResourcePtr> m_stencilMeshCache;
	NodePtr m_compositeFrameEventNode;

	friend class CompositorNodeGraphFactory;
};

class CompositorNodeGraphFactory : public TypedNodeGraphFactory<CompositorNodeGraph>
{
public:
	CompositorNodeGraphFactory() = default;

	virtual NodeGraphPtr initialCreateNodeGraph(class IGlWindow* ownerWindow) const override;
};
