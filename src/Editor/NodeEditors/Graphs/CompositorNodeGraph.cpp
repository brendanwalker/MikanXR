#include "CompositorNodeGraph.h"
#include "SdlCommon.h"
#include "GlFrameBuffer.h"
#include "GlMaterial.h"
#include "IMkShader.h"
#include "MikanRenderModelResource.h"
#include "MkScopedObjectBinding.h"
#include "MikanModelResourceManager.h"
#include "GlMaterialInstance.h"
#include "GlStateStack.h"
#include "IMkTriangulatedMesh.h"
#include "MikanShaderCache.h"
#include "MikanTextureCache.h"
#include "IMkVertexDefinition.h"
#include "IMkWindow.h"
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
#include "Nodes/ClientColorTextureNode.h"
#include "Nodes/ClientDepthTextureNode.h"
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
	addNodeFactory<ClientColorTextureNodeFactory>();
	addNodeFactory<ClientDepthTextureNodeFactory>();
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

	// Create triangulated mesh used to render the layer onto
	bool bSuccess = createLayerQuadMeshes();

	// Create meshes used to draw quad and box stencils and depth masks
	bSuccess &= createQuadMeshes();
	bSuccess &= createBoxMeshes();

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
	m_depthBoxMesh = nullptr;
	m_depthQuadMesh = nullptr;
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

		// Create a scoped binding for the video export framebuffer
		MkScopedObjectBinding compositorFramebufferBinding(
			*evaluator.getCurrentWindow()->getGlStateStack().getCurrentState(),
			"Compositor Framebuffer Scope",
			m_compositingFrameBuffer);
		if (compositorFramebufferBinding)
		{
			// Turn off depth testing for compositing
			compositorFramebufferBinding.getGlState().disableFlag(eGlStateFlagType::depthTest);

			// Evaluate the composite frame nodes
			evaluator.evaluateFlowPinChain(m_compositeFrameEventNode);
		}
		else
		{
			evaluator.addError(
				NodeEvaluationError(
					eNodeEvaluationErrorCode::evaluationError,
					"Broken frame buffer"));
		}
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
	return m_compositingFrameBuffer ? m_compositingFrameBuffer->getColorTexture() : GlTextureConstPtr();
}

void CompositorNodeGraph::setExternalCompositedFrameTexture(GlTexturePtr externalTexture)
{
	m_compositingFrameBuffer->setExternalColorTexture(externalTexture);
}

MikanRenderModelResourcePtr CompositorNodeGraph::getOrLoadStencilRenderModel(
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
		// Load the stencil model and render it using the flat textured material
		auto stencilMaterial= 
			getOwnerWindow()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PT_TEXTURED);
		auto renderModelPtr= 
			getOwnerWindow()->getModelResourceManager()->fetchRenderModel(
				stencilDefinition->getModelPath(), stencilMaterial);

		if (renderModelPtr)
		{
			m_stencilMeshCache.insert({stencilId, renderModelPtr});
		}
	}

	return MikanRenderModelResourcePtr();
}

MikanRenderModelResourcePtr CompositorNodeGraph::getOrLoadDepthRenderModel(ModelStencilDefinitionPtr stencilDefinition)
{
	MikanStencilID stencilId = stencilDefinition->getStencilId();
	auto it = m_depthMeshCache.find(stencilId);

	if (it != m_depthMeshCache.end())
	{
		return it->second;
	}
	else
	{
		// Load the depth model and render it using the simple depth material
		auto depthMaterial =
			getOwnerWindow()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_P_LINEAR_DEPTH);
		auto renderModelPtr =
			getOwnerWindow()->getModelResourceManager()->fetchRenderModel(
				stencilDefinition->getModelPath(), depthMaterial);

		if (renderModelPtr)
		{
			m_depthMeshCache.insert({stencilId, renderModelPtr});
		}
	}

	return MikanRenderModelResourcePtr();
}

void CompositorNodeGraph::flushStencilRenderModel(MikanStencilID stencilId)
{
	{
		auto it = m_stencilMeshCache.find(stencilId);
		if (it != m_stencilMeshCache.end())
		{
			// Remove the entry from the stencil id -> stencil render resource table
			m_stencilMeshCache.erase(it);
		}
	}

	{
		auto it = m_depthMeshCache.find(stencilId);
		if (it != m_depthMeshCache.end())
		{
			// Remove the entry from the stencil id -> depth render resource table
			m_depthMeshCache.erase(it);
		}
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
			// Flush the models we have loaded for the given stencil.
			// We'll reload it next time the compositor renders the stencil.
			flushStencilRenderModel(modelStencilConfig->getStencilId());
		}
	}
}

bool CompositorNodeGraph::createLayerQuadMeshes()
{
	static uint16_t x_indices[] = {0, 1, 2, 0, 2, 3};

	auto material = getOwnerWindow()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PT_FULLSCREEN_RGB_TEXTURE);
	assert(material);

	struct QuadVertex
	{
		glm::vec2 aPos;
		glm::vec2 aTexCoords;
	};
	size_t vertexSize = sizeof(QuadVertex);

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
				getOwnerWindow(),
				"layer_quad_mesh",
				(const uint8_t*)x_vertices,
				vertexSize,
				4, // 4 verts
				(const uint8_t*)x_indices,
				sizeof(uint16_t), // 2 bytes per index
				2, // 2 tris
				false); // mesh doesn't own quad vert data

		if (!m_layerMesh->setMaterial(material) ||
			!m_layerMesh->createResources())
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::createLayerQuadMeshes()") << "Failed to create layer mesh";
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
				getOwnerWindow(),
				"layer_vflipped_quad_mesh",
				(const uint8_t*)x_vertices,
				vertexSize,
				4, // 4 verts
				(const uint8_t*)x_indices,
				sizeof(uint16_t), // 2 bytes per index
				2, // 2 tris
				false); // mesh doesn't own quad vert data

		if (!m_layerVFlippedMesh->setMaterial(material) ||
			!m_layerVFlippedMesh->createResources())
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::createLayerQuadMeshes()") << "Failed to create vflipped layer mesh";
			return false;
		}
	}

	return true;
}

bool CompositorNodeGraph::createQuadMeshes()
{
	auto stencilMaterial = getOwnerWindow()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_P_SOLID_COLOR);
	assert(stencilMaterial);
	auto depthMaterial = getOwnerWindow()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_P_LINEAR_DEPTH);
	assert(depthMaterial);

	struct QuadlVertex
	{
		glm::vec3 aPos;
	};
	size_t vertexSize = sizeof(QuadlVertex);

	// Create triangulated quad mesh to draw the quad stencils
	{
		static QuadlVertex x_vertices[] = {
			//   positions
			{glm::vec3(-0.5f,  0.5f, 0.0f)},
			{glm::vec3(-0.5f, -0.5f, 0.0f)},
			{glm::vec3(0.5f, -0.5f, 0.0f)},
			{glm::vec3(0.5f,  0.5f, 0.0f)},
		};
		static uint16_t x_indices[] = {0, 1, 2, 0, 2, 3};

		m_stencilQuadMesh = std::make_shared<GlTriangulatedMesh>(
			getOwnerWindow(),
			"quad_stencil_mesh",
			(const uint8_t*)x_vertices,
			vertexSize,
			4, // 4 verts in a quad
			(const uint8_t*)x_indices,
			sizeof(uint16_t), // 2 bytes per index
			2, // 2 tris in a quad
			false); // mesh doesn't own quad vertex data

		if (!m_stencilQuadMesh->setMaterial(stencilMaterial) ||
			!m_stencilQuadMesh->createResources())
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::createQuadMeshes()") << "Failed to create quad stencil mesh";
			return false;
		}

		m_depthQuadMesh = std::make_shared<GlTriangulatedMesh>(
			getOwnerWindow(),
			"quad_depth_mesh",
			(const uint8_t*)x_vertices,
			vertexSize,
			4, // 4 verts in a quad
			(const uint8_t*)x_indices,
			sizeof(uint16_t), // 2 bytes per index
			2, // 2 tris in a quad
			false); // mesh doesn't own quad vertex data

		if (!m_depthQuadMesh->setMaterial(depthMaterial) ||
			!m_depthQuadMesh->createResources())
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::createQuadMeshes()") << "Failed to create quad depth mesh";
			return false;
		}
	}

	return true;
}

bool CompositorNodeGraph::createBoxMeshes()
{
	auto stencilMaterial = getOwnerWindow()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_P_SOLID_COLOR);
	assert(stencilMaterial);
	auto depthMaterial = getOwnerWindow()->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_P_LINEAR_DEPTH);
	assert(depthMaterial);

	struct BoxVertex
	{
		glm::vec3 aPos;
	};
	size_t vertexSize = sizeof(BoxVertex);

	// Create triangulated box mesh to draw the box stencils
	{
		static BoxVertex x_vertices[] = {
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
			getOwnerWindow(),
			"box_stencil_mesh",
			(const uint8_t*)x_vertices,
			vertexSize,
			8, // 8 verts in a cube
			(const uint8_t*)x_indices,
			sizeof(uint16_t), // 2 bytes per index
			12, // 12 tris in a cube (6 faces * 2 tris/face)
			false); // mesh doesn't own box vertex data
		if (!m_stencilBoxMesh->setMaterial(stencilMaterial) ||
			!m_stencilBoxMesh->createResources())
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::createBoxMeshes()") << "Failed to create box stencil mesh";
			return false;
		}

		m_depthBoxMesh = std::make_shared<GlTriangulatedMesh>(
			getOwnerWindow(),
			"box_depth_mesh",
			(const uint8_t*)x_vertices,
			vertexSize,
			8, // 8 verts in a cube
			(const uint8_t*)x_indices,
			sizeof(uint16_t), // 2 bytes per index
			12, // 12 tris in a cube (6 faces * 2 tris/face)
			false); // mesh doesn't own box vertex data
		if (!m_depthBoxMesh->setMaterial(depthMaterial) ||
			!m_depthBoxMesh->createResources())
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::createBoxMeshes()") << "Failed to create box stencil mesh";
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
	if (!m_compositingFrameBuffer->isValid())
	{
		m_compositingFrameBuffer->createResources();
	}
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