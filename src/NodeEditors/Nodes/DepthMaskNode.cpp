#include "DepthMaskNode.h"
#include "GlFrameCompositor.h"
#include "GlFrameBuffer.h"
#include "GlMaterial.h"
#include "GlRenderModelResource.h"
#include "GlModelResourceManager.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlStateStack.h"
#include "GlTexture.h"
#include "GlTriangulatedMesh.h"
#include "GlMaterialInstance.h"
#include "IGlWindow.h"
#include "Logger.h"
#include "MainWindow.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"
#include "VideoSourceView.h"

#include "QuadStencilComponent.h"
#include "BoxStencilComponent.h"
#include "ModelStencilComponent.h"
#include "StencilObjectSystem.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include "Pins/ArrayPin.h"
#include "Pins/TexturePin.h"

#include "Properties/GraphMaterialProperty.h"
#include "Properties/GraphStencilProperty.h"
#include "Properties/GraphTextureProperty.h"

#include "imgui.h"
#include "imnodes.h"

#include <glm/gtc/type_ptr.hpp>

#include <typeinfo>
#include <GL/glew.h>

#include <easy/profiler.h>

// -- DepthMaskNodeConfig -----
configuru::Config DepthMaskNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["disable_quad_stencils"] = bDisableQuadStencils;
	pt["disable_box_stencils"] = bDisableBoxStencils;
	pt["disable_model_stencils"] = bDisableModelStencils;

	return pt;
}

void DepthMaskNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	bDisableQuadStencils = pt.get_or<bool>("disable_quad_stencils", false);
	bDisableBoxStencils = pt.get_or<bool>("disable_box_stencils", false);
	bDisableModelStencils = pt.get_or<bool>("disable_model_stencils", false);
}

// -- DepthMaskNode -----
DepthMaskNode::DepthMaskNode() : Node()
{
	// Create the frame buffer, but don't init its internals yet.
	// Wait until evaluation to get the right output texture size.
	m_depthFrameBuffer = std::make_shared<GlFrameBuffer>("DepthMask Frame Buffer");
	m_depthFrameBuffer->setAttachmentType(GlFrameBuffer::eAttachmentType::DEPTH);
}

DepthMaskNode::~DepthMaskNode()
{
	// Clean up the frame buffer
	m_depthFrameBuffer = nullptr;

	// Free pin references
	m_stencilsPin = nullptr;

	// Stop listening to events from owner graph
	setOwnerGraph(NodeGraphPtr());
}

bool DepthMaskNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto depthMaskNodeConfig  = std::static_pointer_cast<const DepthMaskNodeConfig>(nodeConfig);

		m_bDisableQuadStencil = depthMaskNodeConfig->bDisableQuadStencils;
		m_bDisableBoxStencil = depthMaskNodeConfig->bDisableBoxStencils;
		m_bDisableModelStencil = depthMaskNodeConfig->bDisableModelStencils;

		return true;
	}

	return false;
}

void DepthMaskNode::onGraphLoaded(bool success)
{
	// Don't bother with setup if the graph load failed
	if (!success)
		return;

	// Make sure we have a stencil input pin
	ArrayPinPtr stencilInPin = getFirstPinOfType<ArrayPin>(eNodePinDirection::INPUT);
	if (stencilInPin && stencilInPin->getElementClassName() == GraphStencilProperty::k_propertyClassName)
	{
		setStencilsPin(stencilInPin);
	}

	// Make sure we have a texture output pin
	setTexturePin(getFirstPinOfType<TexturePin>(eNodePinDirection::OUTPUT));
}

void DepthMaskNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto depthMaskNodeConfig = std::static_pointer_cast<DepthMaskNodeConfig>(nodeConfig);

	depthMaskNodeConfig->bDisableQuadStencils = m_bDisableQuadStencil;
	depthMaskNodeConfig->bDisableBoxStencils = m_bDisableBoxStencil;
	depthMaskNodeConfig->bDisableModelStencils = m_bDisableModelStencil;

	Node::saveToConfig(nodeConfig);
}

void DepthMaskNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnGraphLoaded -= MakeDelegate(this, &DepthMaskNode::onGraphLoaded);
			m_ownerGraph = nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnGraphLoaded += MakeDelegate(this, &DepthMaskNode::onGraphLoaded);
			m_ownerGraph = newOwnerGraph;
		}
	}
}

void DepthMaskNode::setStencilsPin(ArrayPinPtr inPin)
{
	m_stencilsPin = inPin;
}

void DepthMaskNode::setTexturePin(TexturePinPtr inPin)
{
	m_texturePin = inPin;
}

bool DepthMaskNode::evaluateNode(NodeEvaluator& evaluator)
{
	bool bSuccess = true;

	if (bSuccess && !evaluateInputs(evaluator))
	{
		bSuccess = false;
	}

	if (bSuccess)
	{
		// Use the current video source's frame size as the depth texture size
		VideoSourceViewPtr videoSource = evaluator.getCurrentVideoSourceView();
		if (videoSource)
		{
			int frameWidth = (int)videoSource->getFrameWidth();
			int frameHeight = (int)videoSource->getFrameHeight();

			// Does nothing if the frame buffer is already the correct size
			// Invalidates the frame buffer if it's not the correct size
			m_depthFrameBuffer->setSize(frameWidth, frameHeight);
		}

		// (Re)Initialize the frame buffer if it's in an invalid state
		if (!m_depthFrameBuffer->isValid())
		{
			if (m_depthFrameBuffer->createResources())
			{
				// Bind the frame buffer's texture to the output texture pin
				m_texturePin->setValue(m_depthFrameBuffer->getTexture());
			}
			else
			{
				m_texturePin->setValue(GlTexturePtr());
				evaluator.addError(
					NodeEvaluationError(
						eNodeEvaluationErrorCode::evaluationError,
						"Unable to create depth frame buffer",
						this));
				bSuccess = false;
			}
		}
	}

	if (bSuccess)
	{
		if (!m_quadStencilIds.empty() || 
			!m_boxStencilIds.empty() ||
			!m_modelStencilIds.empty())
		{
			GlScopedState glStateScope = evaluator.getCurrentWindow()->getGlStateStack().createScopedState();
			GlState& glState = glStateScope.getStackState();

			// Bind the frame buffer that we render the depth mask into
			m_depthFrameBuffer->bindFrameBuffer(glState);
			
			// Apply any Stencils assigned to the node
			evaluateQuadStencils(glState);
			evaluateBoxStencils(glState);
			evaluateModelStencils(glState);

			// Unbind the depth frame buffer now that rendering is done
			m_depthFrameBuffer->unbindFrameBuffer();
		}
		else
		{
			evaluator.addError(
				NodeEvaluationError(
					eNodeEvaluationErrorCode::missingInput,
					"No stencils",
					this));
			bSuccess = false;
		}
	}

	return bSuccess;
}

void DepthMaskNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// title bar
	if (NodeEditorUI::DrawPropertySheetHeader("Depth Mask Node"))
	{
		NodeEditorUI::DrawCheckBoxProperty(
			"DepthMaskNodeDisableQuadStencil",
			"Disable Quads",
			m_bDisableQuadStencil);
		NodeEditorUI::DrawCheckBoxProperty(
			"DepthMaskNodeDisableBoxStencil",
			"Disable Boxes",
			m_bDisableBoxStencil);
		NodeEditorUI::DrawCheckBoxProperty(
			"DepthMaskNodeDisableModelStencil",
			"Disable Models",
			m_bDisableModelStencil);

	}
}

void DepthMaskNode::onLinkConnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_stencilsPin)
	{
		m_stencilsPin->copyValueFromSourcePin();

		rebuildStencilLists();
	}
}

void DepthMaskNode::onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_stencilsPin)
	{
		m_stencilsPin->clearArray();

		rebuildStencilLists();
	}
}

void DepthMaskNode::rebuildStencilLists()
{
	m_quadStencilIds.clear();
	m_boxStencilIds.clear();
	m_modelStencilIds.clear();

	if (m_stencilsPin)
	{
		for (GraphPropertyPtr property : m_stencilsPin->getArray())
		{
			if (!property)
				continue;

			if (property->getClassName() != GraphStencilProperty::k_propertyClassName)
				continue;

			auto stencilProperty = std::static_pointer_cast<GraphStencilProperty>(property);
			StencilComponentPtr stencilComponent = stencilProperty->getStencilComponent();
			if (!stencilComponent)
				continue;

			if (auto quadStencil = std::dynamic_pointer_cast<QuadStencilComponent>(stencilComponent))
			{
				m_quadStencilIds.push_back(quadStencil->getStencilComponentDefinition()->getStencilId());
			}
			else if (auto boxStencil = std::dynamic_pointer_cast<BoxStencilComponent>(stencilComponent))
			{
				m_boxStencilIds.push_back(boxStencil->getStencilComponentDefinition()->getStencilId());
			}
			else if (auto modelStencil = std::dynamic_pointer_cast<ModelStencilComponent>(stencilComponent))
			{
				m_modelStencilIds.push_back(modelStencil->getStencilComponentDefinition()->getStencilId());
			}
		}
	}
}

void DepthMaskNode::evaluateQuadStencils(GlState& glState)
{
	EASY_FUNCTION();

	if (m_bDisableQuadStencil)
		return;

	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());
	GlTriangulatedMeshPtr stencilQuadMesh = compositorGraph->getStencilQuadMesh();

	GlFrameCompositor* frameCompositor = MainWindow::getInstance()->getFrameCompositor();
	if (!frameCompositor)
		return;

	// Get the camera pose matrix for the current tracked video source
	glm::mat4 cameraXform;
	if (!frameCompositor->getVideoSourceCameraPose(cameraXform))
		return;

	// Also get the the view-projection matrix for the tracked video source
	// (camera view + projection xform used by stencil shader)
	glm::mat4 vpMatrix;
	if (!frameCompositor->getVideoSourceViewProjection(vpMatrix))
		return;

	// Collect stencil in view of the tracked camera
	const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
	const glm::vec3 cameraPosition(cameraXform[3]);

	std::vector<QuadStencilComponentPtr> quadStencilList;
	StencilObjectSystem::getSystem()->getRelevantQuadStencilList(
		&m_quadStencilIds,
		cameraPosition,
		cameraForward,
		quadStencilList);

	if (quadStencilList.size() == 0)
		return;

	GlMaterialInstancePtr materialInstance= stencilQuadMesh->getMaterialInstance();
	GlMaterialConstPtr material= materialInstance->getMaterial();

	if (auto materialBinding = material->bindMaterial())
	{
		// Draw stencil quads first
		for (QuadStencilComponentPtr stencil : quadStencilList)
		{
			// Set the model matrix of stencil quad
			auto stencilConfig = stencil->getQuadStencilDefinition();
			const glm::mat4 xform = stencil->getWorldTransform();
			const glm::vec3 x_axis = glm::vec3(xform[0]) * stencilConfig->getQuadWidth();
			const glm::vec3 y_axis = glm::vec3(xform[1]) * stencilConfig->getQuadHeight();
			const glm::vec3 z_axis = glm::vec3(xform[2]);
			const glm::vec3 position = glm::vec3(xform[3]);
			const glm::mat4 modelMatrix =
				glm::mat4(
					glm::vec4(x_axis, 0.f),
					glm::vec4(y_axis, 0.f),
					glm::vec4(z_axis, 0.f),
					glm::vec4(position, 1.f));

			// Set the model-view-projection matrix on the stencil shader
			materialInstance->setMat4BySemantic(
				eUniformSemantic::modelViewProjectionMatrix,
				vpMatrix * modelMatrix);

			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				// Draw the quad
				stencilQuadMesh->drawElements();
			}
		}
	}
}

void DepthMaskNode::evaluateBoxStencils(GlState& glState)
{
	EASY_FUNCTION();

	if (m_bDisableBoxStencil)
		return;

	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());
	GlTriangulatedMeshPtr stencilBoxMesh = compositorGraph->getStencilBoxMesh();

	GlFrameCompositor* frameCompositor = MainWindow::getInstance()->getFrameCompositor();
	if (!frameCompositor)
		return;

	// Get the camera pose matrix for the current tracked video source
	glm::mat4 cameraXform;
	if (!frameCompositor->getVideoSourceCameraPose(cameraXform))
		return;

	// Also get the the view-projection matrix for the tracked video source
	// (camera view + projection xform used by stencil shader)
	glm::mat4 vpMatrix;
	if (!frameCompositor->getVideoSourceViewProjection(vpMatrix))
		return;

	// Collect stencil in view of the tracked camera
	const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
	const glm::vec3 cameraPosition(cameraXform[3]);

	std::vector<BoxStencilComponentPtr> boxStencilList;
	StencilObjectSystem::getSystem()->getRelevantBoxStencilList(
		&m_boxStencilIds,
		cameraPosition,
		cameraForward,
		boxStencilList);

	if (boxStencilList.size() == 0)
		return;

	GlMaterialInstancePtr materialInstance = stencilBoxMesh->getMaterialInstance();
	GlMaterialConstPtr material = materialInstance->getMaterial();

	// Then draw stencil boxes ...
	if (auto materialBinding = material->bindMaterial())
	{
		for (BoxStencilComponentPtr stencil : boxStencilList)
		{
			// Set the model matrix of stencil quad
			auto stencilConfig = stencil->getBoxStencilDefinition();
			const glm::mat4 xform = stencil->getWorldTransform();
			const glm::vec3 x_axis = glm::vec3(xform[0]) * stencilConfig->getBoxXSize();
			const glm::vec3 y_axis = glm::vec3(xform[1]) * stencilConfig->getBoxYSize();
			const glm::vec3 z_axis = glm::vec3(xform[2]) * stencilConfig->getBoxZSize();
			const glm::vec3 position = glm::vec3(xform[3]);
			const glm::mat4 modelMatrix =
				glm::mat4(
					glm::vec4(x_axis, 0.f),
					glm::vec4(y_axis, 0.f),
					glm::vec4(z_axis, 0.f),
					glm::vec4(position, 1.f));

			// Set the model-view-projection matrix on the stencil shader
			materialInstance->setMat4BySemantic(
				eUniformSemantic::modelViewProjectionMatrix,
				vpMatrix * modelMatrix);

			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				// Draw the box
				stencilBoxMesh->drawElements();
			}
		}
	}
}

void DepthMaskNode::evaluateModelStencils(GlState& glState)
{
	EASY_FUNCTION();

	if (m_bDisableModelStencil)
		return;

	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());

	GlFrameCompositor* frameCompositor = MainWindow::getInstance()->getFrameCompositor();
	if (!frameCompositor)
		return;

	// Get the camera pose matrix for the current tracked video source
	glm::mat4 cameraXform;
	if (!frameCompositor->getVideoSourceCameraPose(cameraXform))
		return;

	// Also get the the view-projection matrix for the tracked video source
	// (camera view + projection xform used by stencil shader)
	glm::mat4 vpMatrix;
	if (!frameCompositor->getVideoSourceViewProjection(vpMatrix))
		return;

	// Collect stencil in view of the tracked camera
	const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
	const glm::vec3 cameraPosition(cameraXform[3]);

	std::vector<ModelStencilComponentPtr> modelStencilList;
	StencilObjectSystem::getSystem()->getRelevantModelStencilList(
		&m_modelStencilIds,
		cameraPosition,
		cameraForward,
		modelStencilList);

	if (modelStencilList.size() == 0)
		return;

	// Then draw stencil models
	for (ModelStencilComponentPtr stencil : modelStencilList)
	{
		auto stencilConfig = stencil->getModelStencilDefinition();
		const MikanStencilID stencilId = stencilConfig->getStencilId();
		GlRenderModelResourcePtr renderModelResource =
			compositorGraph->getOrLoadStencilRenderModel(stencilConfig);

		if (renderModelResource)
		{
			// Set the model matrix of stencil model
			const glm::mat4 modelMatrix = stencil->getWorldTransform();
			const glm::mat4 mvpMatrix = vpMatrix * modelMatrix;

			for (int meshIndex = 0; meshIndex < renderModelResource->getTriangulatedMeshCount(); ++meshIndex)
			{
				GlTriangulatedMeshPtr mesh = renderModelResource->getTriangulatedMesh(meshIndex);
				GlMaterialInstancePtr materialInst = mesh->getMaterialInstance();
				GlMaterialConstPtr material = materialInst->getMaterial();

				// Set the model-view-projection matrix on the stencil material instance
				materialInst->setMat4BySemantic(eUniformSemantic::modelViewProjectionMatrix, mvpMatrix);

				// Bind the material and draw the mesh
				// TODO: We could optimize this by sorting on the shader program
				// to minimize the number of shader program switches
				// Or switch to using a GlScene for rendering stencils,
				// which handles the shader program sorting for us.
				auto materialBinding = material->bindMaterial();
				if (materialBinding)
				{
					auto materialInstanceBinding = materialInst->bindMaterialInstance(materialBinding);
					if (materialInstanceBinding)
					{
						mesh->drawElements();
					}
				}
			}
		}
	}
}

// -- ProgramNode Factory -----
NodePtr DepthMaskNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and default pins
	// The rest of the input pins can't be connected until we have a material assigned
	auto node = std::static_pointer_cast<DepthMaskNode>(NodeFactory::createNode(editorState));

	// Create stencil input pins
	ArrayPinPtr stencilListInPin = node->addPin<ArrayPin>("stencils", eNodePinDirection::INPUT);
	stencilListInPin->setElementClassName(GraphStencilProperty::k_propertyClassName);
	stencilListInPin->setHasDefaultValue(true); // Unconnected array pin == no stencils
	node->setStencilsPin(stencilListInPin);

	// Create texture output pin
	auto outputPin = node->addPin<TexturePin>("texture", eNodePinDirection::OUTPUT);
	outputPin->editorSetShowPinName(false);
	node->setTexturePin(outputPin);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the default pins to a compatible target pin
	autoConnectInputPin(editorState, stencilListInPin);
	autoConnectOutputPin(editorState, outputPin);

	return node;
}