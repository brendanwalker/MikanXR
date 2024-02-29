#include "CompositorNodeGraph.h"
#include "GlCommon.h"
#include "GlFrameBuffer.h"
#include "GlProgram.h"
#include "GlRenderModelResource.h"
#include "GlModelResourceManager.h"
#include "GlMaterialInstance.h"
#include "GlStateStack.h"
#include "GlTriangulatedMesh.h"
#include "GlShaderCache.h"
#include "GlTextureCache.h"
#include "GlVertexDefinition.h"
#include "IGlWindow.h"
#include "Logger.h"
#include "ModelStencilComponent.h"
#include "StencilObjectSystem.h"
#include "VideoSourceView.h"

// Assets References
#include "ModelAssetReference.h"
#include "MaterialAssetReference.h"
#include "TextureAssetReference.h"

// Properties
#include "Properties/GraphBoolProperty.h"
#include "Properties/GraphMaterialProperty.h"
#include "Properties/GraphModelProperty.h"
#include "Properties/GraphStencilProperty.h"
#include "Properties/GraphTextureProperty.h"

// Nodes
#include "Nodes/ArrayNode.h"
#include "Nodes/ClientTextureNode.h"
#include "Nodes/DepthMaskNode.h"
#include "Nodes/DrawLayerNode.h"
#include "Nodes/EventNode.h"
#include "Nodes/MaterialNode.h"
#include "Nodes/MousePosNode.h"
#include "Nodes/StencilNode.h"
#include "Nodes/TextureNode.h"
#include "Nodes/TimeNode.h"
#include "Nodes/VideoTextureNode.h"

// Graph
#include "NodeEditorState.h"
#include "Graphs/NodeEvaluator.h"

#include <easy/profiler.h>

// -- CompositorNodeGraph -----
const std::string CompositorNodeGraph::k_compositeFrameEventName= "OnCompositeFrame";

CompositorNodeGraph::CompositorNodeGraph() : NodeGraph()
{
	// Assets this graph can reference
	addAssetReferenceFactory<MaterialAssetReferenceFactory>();
	addAssetReferenceFactory<TextureAssetReferenceFactory>();

	// Add property types this graph can use
	addPropertyFactory<GraphBoolPropertyFactory>();
	addPropertyFactory<GraphMaterialPropertyFactory>();
	addPropertyFactory<GraphTexturePropertyFactory>();
	addPropertyFactory<GraphStencilPropertyFactory>();

	// Nodes this graph can spawn
	addNodeFactory<ArrayNodeFactory>();
	addNodeFactory<ClientTextureNodeFactory>();
	addNodeFactory<DrawLayerNodeFactory>();
	addNodeFactory<DepthMaskNodeFactory>();
	addNodeFactory<EventNodeFactory>();
	addNodeFactory<MousePosNodeFactory>();
	addNodeFactory<MaterialNodeFactory>();
	addNodeFactory<StencilNodeFactory>();
	addNodeFactory<TextureNodeFactory>();
	addNodeFactory<TimeNodeFactory>();
	addNodeFactory<VideoTextureNodeFactory>();
}

bool CompositorNodeGraph::createResources()
{
	assert(getOwnerWindow());

	// Create the frame buffer, but don't init it's resources yet
	m_compositingFrameBuffer = std::make_shared<GlFrameBuffer>("Compositing Node Graph Frame Buffer");

	// Start listening for Model stencil changes
	StencilObjectSystem::getSystem()->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &CompositorNodeGraph::onStencilSystemConfigMarkedDirty);

	// Create rendering resources
	bool bSuccess = createStencilShaders();

	// Create triangulated mesh used to render the layer onto
	bSuccess &= createLayerQuadMeshes();

	// Create meshes used to draw quad and box stencils
	bSuccess &= createStencilMeshes();

	return bSuccess;
}

void CompositorNodeGraph::disposeResources()
{
	// Clean up the frame buffer
	m_compositingFrameBuffer = nullptr;

	// Stop listening for Model stencil changes
	StencilObjectSystem::getSystem()->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &CompositorNodeGraph::onStencilSystemConfigMarkedDirty);

	// Free rendering resources
	m_stencilBoxMesh = nullptr;
	m_stencilQuadMesh = nullptr;
	m_layerVFlippedMesh = nullptr;
	m_layerMesh = nullptr;
}

bool CompositorNodeGraph::loadFromConfig(const NodeGraphConfig& config)
{
	bool bSuccess= true;

	if (NodeGraph::loadFromConfig(config))
	{
		bSuccess= bindEventNodes();
	}
	else
	{
		MIKAN_LOG_ERROR("CompositorNodeGraph::loadFromConfig") << "Failed to parse node graph config";
		bSuccess= false;
	}

	return bSuccess;
}

bool CompositorNodeGraph::bindEventNodes()
{
	bool bSuccess = true;

	m_compositeFrameEventNode = getEventNodeByName(k_compositeFrameEventName);
	if (!m_compositeFrameEventNode)
	{
		MIKAN_LOG_ERROR("CompositorNodeGraph::bindEventNodes")
			<< "Failed to find event node: " << k_compositeFrameEventName;
		bSuccess = false;
	}

	return bSuccess;
}

bool CompositorNodeGraph::compositeFrame(NodeEvaluator& evaluator)
{
	EASY_FUNCTION();

	if (m_compositeFrameEventNode)
	{
		// Make sure the frame buffer is the correct size
		updateCompositingFrameBufferSize(evaluator);

		// Turn off depth testing for compositing
		GlScopedState updateCompositeGlStateScope = evaluator.getCurrentWindow()->getGlStateStack().createScopedState();
		updateCompositeGlStateScope.getStackState().disableFlag(eGlStateFlagType::depthTest);

		// Bind the frame buffer that we render the composited frame to
		m_compositingFrameBuffer->bindFrameBuffer(updateCompositeGlStateScope.getStackState());

		// Evaluate the composite frame nodes
		evaluator.evaluateFlowPinChain(m_compositeFrameEventNode);

		// Unbind the layer frame buffer
		m_compositingFrameBuffer->unbindFrameBuffer();
	}
	else
	{
		evaluator.addError(
			NodeEvaluationError(
				eNodeEvaluationErrorCode::invalidNode, 
				"Missing compositeFrame event node"));
	}

	return !evaluator.hasErrors();
}

GlTextureConstPtr CompositorNodeGraph::getCompositedFrameTexture() const
{
	return m_compositingFrameBuffer->getTexture();
}

void CompositorNodeGraph::setExternalCompositedFrameTexture(GlTexturePtr externalTexture)
{
	m_compositingFrameBuffer->setExternalTexture(externalTexture);
}

GlRenderModelResourcePtr CompositorNodeGraph::getOrLoadStencilRenderModel(
	ModelStencilDefinitionPtr stencilDefinition)
{
	MikanStencilID stencilId= stencilDefinition->getStencilId();
	auto it = m_stencilMeshCache.find(stencilId);

	if (it != m_stencilMeshCache.end())
	{
		return it->second;
	}
	else
	{
		auto renderModelPtr= StencilObjectSystem::loadStencilRenderModel(getOwnerWindow(), stencilDefinition);
		if (renderModelPtr)
		{
			m_stencilMeshCache.insert({stencilId, renderModelPtr});
		}
	}

	return GlRenderModelResourcePtr();
}

void CompositorNodeGraph::flushStencilRenderModel(MikanStencilID stencilId)
{
	auto it = m_stencilMeshCache.find(stencilId);

	if (it != m_stencilMeshCache.end())
	{
		// updateStencils() will reload the meshes if the model path is still valid for this stencil
		m_stencilMeshCache.erase(it);
	}
}

void CompositorNodeGraph::onStencilSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	ModelStencilDefinitionPtr modelStencilConfig = std::dynamic_pointer_cast<ModelStencilDefinition>(configPtr);

	if (modelStencilConfig != nullptr)
	{
		if (changedPropertySet.hasPropertyName(ModelStencilDefinition::k_modelStencilObjPathPropertyId))
		{
			// Flush the model we have loaded for the given stencil.
			// We'll reload it next time the compositor renders the stencil.
			flushStencilRenderModel(modelStencilConfig->getStencilId());
		}
	}
}

bool CompositorNodeGraph::createStencilShaders()
{
	m_vertexOnlyStencilShader =
		getOwnerWindow()->getShaderCache()->fetchCompiledGlProgram(
			StencilObjectSystem::getVertexOnlyStencilShaderCode());
	if (!m_vertexOnlyStencilShader)
	{
		MIKAN_LOG_ERROR("DrawLayerNode::createStencilShader()") << "Failed to compile vertex only stencil shader";
		return false;
	}

	return true;
}

const GlVertexDefinition& CompositorNodeGraph::getLayerQuadVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const int32_t vertexSize = (int32_t)sizeof(CompositorNodeGraph::QuadVertex);
		std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

		attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position2f, false, vertexSize, offsetof(CompositorNodeGraph::QuadVertex, aPos)));
		attribs.push_back(GlVertexAttribute(1, eVertexSemantic::texel2f, false, vertexSize, offsetof(CompositorNodeGraph::QuadVertex, aTexCoords)));

		x_vertexDefinition.vertexSize = vertexSize;
	}

	return x_vertexDefinition;
}

bool CompositorNodeGraph::createLayerQuadMeshes()
{
	static uint16_t x_indices[] = {0, 1, 2, 0, 2, 3};

	// Create triangulated quad mesh to draw the layer on
	{
		static QuadVertex x_vertices[] = {
			//   positions                texCoords
				{glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f)},
				{glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
				{glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
				{glm::vec2(1.0f,  1.0f), glm::vec2(1.0f, 1.0f)},
		};

		m_layerMesh = 
			std::make_shared<GlTriangulatedMesh>(
				"layer_quad_mesh",
				getLayerQuadVertexDefinition(),
				(const uint8_t*)x_vertices,
				4, // 4 verts
				(const uint8_t*)x_indices,
				sizeof(uint16_t), // 2 bytes per index
				2, // 2 tris
				false); // mesh doesn't own quad vert data
		if (!m_layerMesh->createResources())
		{
			MIKAN_LOG_ERROR("DrawLayerNode::createLayerQuadMeshes()") << "Failed to create layer mesh";
			return false;
		}
	}

	// Create triangulated quad mesh to draw the layer on with flipped v texel coordinates
	// (this is usually used for video sources where we need to flip the image vertically)
	{
		static QuadVertex x_vertices[] = {
			//   positions                texCoords (flipped v coordinated)
			{glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 0.0f)},
			{glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 1.0f)},
			{glm::vec2(1.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
			{glm::vec2(1.0f,  1.0f), glm::vec2(1.0f, 0.0f)},
		};

		m_layerVFlippedMesh = 
			std::make_shared<GlTriangulatedMesh>(
				"layer_vflipped_quad_mesh",
				getLayerQuadVertexDefinition(),
				(const uint8_t*)x_vertices,
				4, // 4 verts
				(const uint8_t*)x_indices,
				sizeof(uint16_t), // 2 bytes per index
				2, // 2 tris
				false); // mesh doesn't own quad vert data
		if (!m_layerVFlippedMesh->createResources())
		{
			MIKAN_LOG_ERROR("DrawLayerNode::createLayerQuadMeshes()") << "Failed to create vflipped layer mesh";
			return false;
		}
	}

	return true;
}

bool CompositorNodeGraph::createStencilMeshes()
{
	// Create triangulated quad mesh to draw the quad stencils
	{
		static StencilObjectSystem::StencilVertex x_vertices[] = {
			//   positions
			{glm::vec3(-0.5f,  0.5f, 0.0f)},
			{glm::vec3(-0.5f, -0.5f, 0.0f)},
			{glm::vec3(0.5f, -0.5f, 0.0f)},
			{glm::vec3(0.5f,  0.5f, 0.0f)},
		};
		static uint16_t x_indices[] = {0, 1, 2, 0, 2, 3};

		m_stencilQuadMesh = std::make_shared<GlTriangulatedMesh>(
			"quad_stencil_mesh",
			StencilObjectSystem::getStencilModelVertexDefinition(),
			(const uint8_t*)x_vertices,
			4, // 4 verts in a quad
			(const uint8_t*)x_indices,
			sizeof(uint16_t), // 2 bytes per index
			2, // 2 tris in a quad
			false); // mesh doesn't own quad vertex data
		if (!m_stencilQuadMesh->createResources())
		{
			MIKAN_LOG_ERROR("DrawLayerNode::createStencilMeshes()") << "Failed to create quad stencil mesh";
			return false;
		}
	}

	// Create triangulated box mesh to draw the box stencils
	{
		static StencilObjectSystem::StencilVertex x_vertices[] = {
			//   positions
			{glm::vec3(-0.5f,  0.5f, -0.5f)},
			{glm::vec3(-0.5f,  0.5f,  0.5f)},
			{glm::vec3(0.5f,  0.5f,  0.5f)},
			{glm::vec3(0.5f,  0.5f, -0.5f)},
			{glm::vec3(-0.5f, -0.5f, -0.5f)},
			{glm::vec3(-0.5f, -0.5f,  0.5f)},
			{glm::vec3(0.5f, -0.5f,  0.5f)},
			{glm::vec3(0.5f, -0.5f, -0.5f)},
		};
		static uint16_t x_indices[] = {
			0, 4, 1, 1, 4, 5, // -X Face
			1, 5, 2, 2, 5, 6, // +Z Face
			2, 6, 3, 3, 6, 7, // +X Face
			0, 3, 7, 7, 4, 0, // -Z Face
			5, 4, 6, 6, 4, 7, // -Y Face
			3, 0, 1, 1, 2, 3  // +Y Face 
		};

		m_stencilBoxMesh = std::make_shared<GlTriangulatedMesh>(
			"box_stencil_mesh",
			StencilObjectSystem::getStencilModelVertexDefinition(),
			(const uint8_t*)x_vertices,
			8, // 8 verts in a cube
			(const uint8_t*)x_indices,
			sizeof(uint16_t), // 2 bytes per index
			12, // 12 tris in a cube (6 faces * 2 tris/face)
			false); // mesh doesn't own box vertex data
		if (!m_stencilBoxMesh->createResources())
		{
			MIKAN_LOG_ERROR("DrawLayerNode::createStencilMeshes()") << "Failed to create box stencil mesh";
			return false;
		}
	}

	return true;
}

void CompositorNodeGraph::updateCompositingFrameBufferSize(NodeEvaluator& evaluator)
{
	// Use the current video source's frame size
	VideoSourceViewPtr videoSource = evaluator.getCurrentVideoSourceView();
	if (videoSource)
	{
		int frameWidth = (int)videoSource->getFrameWidth();
		int frameHeight = (int)videoSource->getFrameHeight();

		// Does nothing if the frame buffer is already the correct size
		m_compositingFrameBuffer->setSize(frameWidth, frameHeight);
	}

	// (Re)Initialize the frame buffer if it's in an invalid state
	m_compositingFrameBuffer->createResources();
}

// -- CompositorNodeGraphFactory ----
NodeGraphPtr CompositorNodeGraphFactory::initialCreateNodeGraph(IGlWindow* ownerWindow) const
{
	auto nodeGraph= NodeGraphFactory::initialCreateNodeGraph(ownerWindow);
	if (!nodeGraph)
	{
		// Something happened with GL resource allocation?
		return NodeGraphPtr();
	}

	auto compositorNodeGraph= std::static_pointer_cast<CompositorNodeGraph>(nodeGraph);

	// Add default nodes
	NodeEditorState editorState;
	editorState.nodeGraph= compositorNodeGraph;

	auto compositeFrameNode= compositorNodeGraph->createTypedNode<EventNode>(editorState);
	compositeFrameNode->setName(CompositorNodeGraph::k_compositeFrameEventName);

	// Store off pointers to the event nodes we manually created
	compositorNodeGraph->bindEventNodes();

	return compositorNodeGraph;
}