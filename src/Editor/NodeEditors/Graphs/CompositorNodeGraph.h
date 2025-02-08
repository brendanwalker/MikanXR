#pragma once

#include "NodeGraph.h"
#include "ComponentFwd.h"
#include "MikanRendererFwd.h"

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
	void setExternalCompositedFrameTexture(IMkTexturePtr externalTexture);

	// Stencil Models
	MikanRenderModelResourcePtr getOrLoadStencilRenderModel(ModelStencilDefinitionPtr stencilDefinition);
	MikanRenderModelResourcePtr getOrLoadDepthRenderModel(ModelStencilDefinitionPtr stencilDefinition);
	void flushStencilRenderModel(MikanStencilID stencilId);
	inline IMkTriangulatedMeshPtr getStencilQuadMesh() const { return m_stencilQuadMesh; }
	inline IMkTriangulatedMeshPtr getStencilBoxMesh() const { return m_stencilBoxMesh; }
	inline IMkTriangulatedMeshPtr getDepthQuadMesh() const { return m_depthQuadMesh; }
	inline IMkTriangulatedMeshPtr getDepthBoxMesh() const { return m_depthBoxMesh; }
	inline IMkTriangulatedMeshPtr getLayerVFlippedMesh() const { return m_layerVFlippedMesh; }
	inline IMkTriangulatedMeshPtr getLayerMesh() const { return m_layerMesh; }

protected:

	bool bindEventNodes();

	bool createLayerQuadMeshes();
	bool createQuadMeshes();
	bool createBoxMeshes();
	void updateCompositingFrameBufferSize(NodeEvaluator& evaluator);

	// Stencil System Events
	void onStencilSystemConfigMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

protected:
	IMkFrameBufferPtr m_compositingFrameBuffer;
	IMkShaderPtr m_vertexOnlyStencilShader;
	IMkTriangulatedMeshPtr m_stencilQuadMesh;
	IMkTriangulatedMeshPtr m_stencilBoxMesh;
	IMkTriangulatedMeshPtr m_depthQuadMesh;
	IMkTriangulatedMeshPtr m_depthBoxMesh;
	IMkTriangulatedMeshPtr m_layerVFlippedMesh;
	IMkTriangulatedMeshPtr m_layerMesh;
	std::map<MikanStencilID, MikanRenderModelResourcePtr> m_stencilMeshCache;
	std::map<MikanStencilID, MikanRenderModelResourcePtr> m_depthMeshCache;
	NodePtr m_compositeFrameEventNode;

	friend class CompositorNodeGraphFactory;
};

class CompositorNodeGraphFactory : public TypedNodeGraphFactory<CompositorNodeGraph>
{
public:
	CompositorNodeGraphFactory() = default;

	virtual NodeGraphPtr initialCreateNodeGraph(class IGlWindow* ownerWindow) const override;
};
