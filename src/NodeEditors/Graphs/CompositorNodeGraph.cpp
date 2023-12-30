#include "CompositorNodeGraph.h"
#include "GlProgram.h"
#include "GlRenderModelResource.h"
#include "GlModelResourceManager.h"
#include "GlShaderCache.h"
#include "GlVertexDefinition.h"
#include "IGlWindow.h"
#include "Logger.h"
#include "ModelStencilComponent.h"
#include "StencilObjectSystem.h"

// Assets References
#include "ModelAssetReference.h"
#include "MaterialAssetReference.h"
#include "TextureAssetReference.h"

// Properties
#include "Properties/GraphVariableList.h"
#include "Properties/GraphMaterialProperty.h"
#include "Properties/GraphModelProperty.h"
#include "Properties/GraphStencilProperty.h"
#include "Properties/GraphTextureProperty.h"

// Nodes
#include "Nodes/ArrayNode.h"
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
	addPropertyFactory<GraphMaterialPropertyFactory>();
	addPropertyFactory<GraphTexturePropertyFactory>();
	addPropertyFactory<GraphStencilPropertyFactory>();

	// Nodes this graph can spawn
	addNodeFactory<ArrayNodeFactory>();
	addNodeFactory<DrawLayerNodeFactory>();
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

	// Start listening for Model stencil changes
	StencilObjectSystem::getSystem()->getStencilSystemConfig()->OnMarkedDirty +=
		MakeDelegate(this, &CompositorNodeGraph::onStencilSystemConfigMarkedDirty);

	// Create rendering resources
	bool bSuccess = createStencilShader();

	return bSuccess;
}

void CompositorNodeGraph::disposeResources()
{
	// Stop listening for Model stencil changes
	StencilObjectSystem::getSystem()->getStencilSystemConfig()->OnMarkedDirty -=
		MakeDelegate(this, &CompositorNodeGraph::onStencilSystemConfigMarkedDirty);

	// Free rendering resources
	m_stencilShader = nullptr;
}

bool CompositorNodeGraph::loadFromConfig(const NodeGraphConfig& config)
{
	bool bSuccess= true;

	if (NodeGraph::loadFromConfig(config))
	{
		m_compositeFrameEventNode= getEventNodeByName(k_compositeFrameEventName);
		if (!m_compositeFrameEventNode)
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::loadFromConfig") 
				<< "Failed to find event node: " << k_compositeFrameEventName;
			bSuccess= false;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("CompositorNodeGraph::loadFromConfig") << "Failed to parse node graph config";
		bSuccess= false;
	}

	return bSuccess;
}

bool CompositorNodeGraph::compositeFrame(NodeEvaluator& evaluator)
{
	EASY_FUNCTION();

	if (m_compositeFrameEventNode)
	{
		evaluator.evaluateFlowPinChain(m_compositeFrameEventNode);
		if (evaluator.getLastErrorCode() != eNodeEvaluationErrorCode::NONE)
		{
			MIKAN_LOG_ERROR("CompositorNodeGraph::compositeFrame - Error: ") << evaluator.getLastErrorMessage();
		}
	}
	else
	{
		evaluator.setLastErrorCode(eNodeEvaluationErrorCode::invalidNode);
		evaluator.setLastErrorMessage("Missing compositeFrame event node");
	}

	return evaluator.getLastErrorCode() == eNodeEvaluationErrorCode::NONE;
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
		if (m_stencilMeshCache.find(stencilId) == m_stencilMeshCache.end())
		{
			const GlVertexDefinition& vertexDefinition = getStencilModelVertexDefinition();
			GlModelResourceManager* modelResourceManager = getOwnerWindow()->getModelResourceManager();

			// It's possible that the model path isn't valid, 
			// in which case renderModelResource will be null.
			// Go ahead an occupy a slot in the m_stencilMeshCache until
			// the entry us explicitly cleared by flushStencilRenderModel.
			GlRenderModelResourcePtr renderModelResource =
				modelResourceManager->fetchRenderModel(
					stencilDefinition->getModelPath(),
					&vertexDefinition);

			m_stencilMeshCache.insert({stencilId, renderModelResource});
		}
	}

	return nullptr;
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

const GlVertexDefinition& CompositorNodeGraph::getStencilModelVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const int32_t vertexSize = (int32_t)sizeof(CompositorNodeGraph::StencilVertex);
		std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

		attribs.push_back(
			GlVertexAttribute(
				0, eVertexSemantic::position3f, false, vertexSize, 
				offsetof(CompositorNodeGraph::StencilVertex, aPos)));

		x_vertexDefinition.vertexSize = vertexSize;
	}

	return x_vertexDefinition;
}

bool CompositorNodeGraph::createStencilShader()
{
	m_stencilShader = getOwnerWindow()->getShaderCache()->fetchCompiledGlProgram(getStencilShaderCode());
	if (!m_stencilShader)
	{
		MIKAN_LOG_ERROR("DrawLayerNode::createStencilShader()") << "Failed to compile stencil shader";
		return false;
	}

	return true;
}

const GlProgramCode* CompositorNodeGraph::getStencilShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"Internal Stencil Shader Code",
		// vertex shader
		R""""(
			#version 330 core
			layout (location = 0) in vec3 aPos;

			uniform mat4 mvpMatrix;

			void main()
			{
				gl_Position = mvpMatrix * vec4(aPos, 1.0);
			}
			)"""",
		//fragment shader
		R""""(
			#version 330 core
			out vec4 FragColor;

			void main()
			{    
				FragColor = vec4(1, 1, 1, 1);
			}
			)"""")
		.addUniform(STENCIL_MVP_UNIFORM_NAME, eUniformSemantic::modelViewProjectionMatrix);

	return &x_shaderCode;
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

	// Add default properties
	compositorNodeGraph->createTypedProperty<GraphVariableList>("materials")
						->assignFactory<GraphMaterialPropertyFactory>();
	compositorNodeGraph->createTypedProperty<GraphVariableList>("stencils")
						->assignFactory<GraphStencilPropertyFactory>();
	compositorNodeGraph->createTypedProperty<GraphVariableList>("textures")
						->assignFactory<GraphTexturePropertyFactory>();

	return compositorNodeGraph;
}