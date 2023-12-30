#include "App.h"
#include "DrawLayerNode.h"
#include "GlFrameCompositor.h"
#include "GlMaterial.h"
#include "GlRenderModelResource.h"
#include "GlModelResourceManager.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlStateStack.h"
#include "GlTexture.h"
#include "GlTriangulatedMesh.h"
#include "IGlWindow.h"
#include "EditorNodeUtil.h"
#include "Logger.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"

#include "QuadStencilComponent.h"
#include "BoxStencilComponent.h"
#include "ModelStencilComponent.h"
#include "StencilObjectSystem.h"

#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include "Pins/ArrayPin.h"
#include "Pins/FloatPin.h"
#include "Pins/FlowPin.h"
#include "Pins/IntPin.h"
#include "Pins/NodePin.h"
#include "Pins/PropertyPin.h"
#include "Pins/TexturePin.h"

#include "Properties/GraphVariableList.h"
#include "Properties/GraphMaterialProperty.h"
#include "Properties/GraphStencilProperty.h"
#include "Properties/GraphTextureProperty.h"

#include "StringUtils.h"

#include "imgui.h"
#include "imnodes.h"

#include <glm/gtc/type_ptr.hpp>

#include <typeinfo>
#include <GL/glew.h>

#include <easy/profiler.h>

// -- DrawLayerNodeConfig -----
configuru::Config DrawLayerNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["blend_mode"] = k_compositorBlendModeStrings[(int)blendMode];
	pt["stencil_mode"] = k_compositorStencilModeStrings[(int)stencilMode];
	pt["vertical_flip"] = bVerticalFlip;
	pt["invert_when_camera_inside"] = bInvertWhenCameraInside;

	return pt;
}

void DrawLayerNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	const std::string blendModeString =
		pt.get_or<std::string>(
			"blend_mode",
			k_compositorBlendModeStrings[(int)eCompositorBlendMode::blendOff]);
	blendMode = StringUtils::FindEnumValue<eCompositorBlendMode>(blendModeString, k_compositorBlendModeStrings);

	const std::string stencilModeString =
		pt.get_or<std::string>(
			"stencil_mode",
			k_stencilTypeStrings[(int)eCompositorStencilMode::insideStencil]);
	stencilMode = StringUtils::FindEnumValue<eCompositorStencilMode>(stencilModeString, k_compositorStencilModeStrings);

	bVerticalFlip = pt.get_or<bool>("vertical_flip", false);
	bInvertWhenCameraInside = pt.get_or<bool>("invert_when_camera_inside", false);
}

// -- DrawLayerNode -----
DrawLayerNode::~DrawLayerNode()
{
	// Free pin references
	m_dynamicMaterialPins.clear();
	m_materialPin= nullptr;
	m_stencilsPin= nullptr;

	// Clean up render resources
	m_material = nullptr;
	m_layerVFlippedMesh= nullptr;
	m_layerMesh= nullptr;
	m_stencilQuadMesh= nullptr;
	m_stencilBoxMesh= nullptr;

	// Stop listening to events from owner graph
	setOwnerGraph(NodeGraphPtr());
}

bool DrawLayerNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto drawLayerNodeConfig = std::static_pointer_cast<const DrawLayerNodeConfig>(nodeConfig);

		m_blendMode= drawLayerNodeConfig->blendMode;
		m_stencilMode= drawLayerNodeConfig->stencilMode;
		m_bVerticalFlip = drawLayerNodeConfig->bVerticalFlip;
		m_bInvertWhenCameraInside = drawLayerNodeConfig->bInvertWhenCameraInside;

		return true;
	}

	return false;
}

void DrawLayerNode::onGraphLoaded(bool success)
{
	if (success)
	{
		// Make sure we have a material input pin
		PropertyPinPtr materialInPin = getFirstPinOfType<PropertyPin>(eNodePinDirection::INPUT);
		if (materialInPin && materialInPin->getPropertyClassName() == GraphMaterialProperty::k_propertyClassName)
		{
			setMaterialPin(materialInPin);
			m_materialPin->copyValueFromSourcePin();

			auto materialProperty = std::dynamic_pointer_cast<GraphMaterialProperty>(m_materialPin->getValue());
			if (materialProperty)
			{
				setMaterial(materialProperty->getMaterialResource());
			}
		}

		// Make sure we have a stencil input pin
		ArrayPinPtr stencilInPin = getFirstPinOfType<ArrayPin>(eNodePinDirection::INPUT);
		if (stencilInPin && stencilInPin->getElementClassName() == GraphStencilProperty::k_propertyClassName)
		{
			setStencilsPin(stencilInPin);
		}

		// Create triangulated mesh used to render the layer onto
		createLayerQuadMeshes();

		// Create meshes used to draw quad and box stencils
		createStencilMeshes();
	}
}

void DrawLayerNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto drawLayerNodeConfig = std::static_pointer_cast<DrawLayerNodeConfig>(nodeConfig);

	drawLayerNodeConfig->blendMode = m_blendMode;
	drawLayerNodeConfig->stencilMode = m_stencilMode;
	drawLayerNodeConfig->bVerticalFlip = m_bVerticalFlip;
	drawLayerNodeConfig->bInvertWhenCameraInside = m_bInvertWhenCameraInside;

	Node::saveToConfig(nodeConfig);
}

void DrawLayerNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnGraphLoaded -= MakeDelegate(this, &DrawLayerNode::onGraphLoaded);
			m_ownerGraph= nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnGraphLoaded += MakeDelegate(this, &DrawLayerNode::onGraphLoaded);
			m_ownerGraph = newOwnerGraph;
		}
	}
}

void DrawLayerNode::setMaterialPin(PropertyPinPtr inPin)
{
	m_materialPin = inPin;
}

void DrawLayerNode::setStencilsPin(ArrayPinPtr inPin)
{
	m_stencilsPin = inPin;
}

void DrawLayerNode::setMaterial(GlMaterialPtr inMaterial)
{
	m_material = inMaterial;
}

bool DrawLayerNode::evaluateNode(NodeEvaluator& evaluator)
{
	bool bSuccess= true;

	if (!m_material)
	{
		evaluator.setLastErrorCode(eNodeEvaluationErrorCode::evaluationError);
		evaluator.setLastErrorMessage("Missing material");
		bSuccess= false;
	}

	if (bSuccess && !evaluateInputs(evaluator))
	{
		bSuccess= false;
	}

	if (bSuccess)
	{
		// Map input pins to the layer material
		for (auto& pin : m_pinsIn)
		{
			if (FloatPinPtr floatPin = std::dynamic_pointer_cast<FloatPin>(pin))
			{
				m_material->setFloatByUniformName(pin->getName(), floatPin->getValue());
			}
			else if (Float2PinPtr float2Pin = std::dynamic_pointer_cast<Float2Pin>(pin))
			{
				m_material->setVec2ByUniformName(pin->getName(), glm::make_vec2(float2Pin->getValue().data()));
			}
			else if (Float3PinPtr float3Pin = std::dynamic_pointer_cast<Float3Pin>(pin))
			{
				m_material->setVec3ByUniformName(pin->getName(), glm::make_vec3(float3Pin->getValue().data()));
			}
			else if (Float4PinPtr float4Pin = std::dynamic_pointer_cast<Float4Pin>(pin))
			{
				m_material->setVec4ByUniformName(pin->getName(), glm::make_vec4(float3Pin->getValue().data()));
			}
			// TODO
			//else if (IntPinPtr floatPin = std::dynamic_pointer_cast<IntPin>(pin))
			//{
			//	m_material->setIntByUniformName(pin->getName(), floatPin->getValue());
			//}
			//else if (Int2PinPtr float2Pin = std::dynamic_pointer_cast<Int2Pin>(pin))
			//{
			//	m_material->setInt2ByUniformName(pin->getName(), glm::make_vec2(float2Pin->getValue().data()));
			//}
			//else if (Int3PinPtr float3Pin = std::dynamic_pointer_cast<Int3Pin>(pin))
			//{
			//	m_material->setInt3ByUniformName(pin->getName(), glm::make_vec3(float3Pin->getValue().data()));
			//}
			//else if (Int4PinPtr float4Pin = std::dynamic_pointer_cast<Int4Pin>(pin))
			//{
			//	m_material->setInt4ByUniformName(pin->getName(), glm::make_vec4(float3Pin->getValue().data()));
			//}			
			//else if (Mat4PinPtr mat4Pin = std::dynamic_pointer_cast<Mat4Pin>(pin))
			//{
			//	m_material->setMat4ByUniformName(pin->getName(), mat4Pin->getValue()));
			//}
			else if (TexturePinPtr texturePin = std::dynamic_pointer_cast<TexturePin>(pin))
			{
				GlTexturePtr texturePtr = texturePin->getValue();
				if (texturePtr)
				{
					m_material->setTextureByUniformName(pin->getName(), texturePtr);
				}
			}
		}

		{
			GlScopedState glStateScope = evaluator.getCurrentWindow()->getGlStateStack().createScopedState();
			GlState& glState = glStateScope.getStackState();

			// Set the blend mode
			switch (m_blendMode)
			{
				case eCompositorBlendMode::blendOff:
					{
						glState.disableFlag(eGlStateFlagType::blend);
					}
					break;
				case eCompositorBlendMode::blendOn:
					{
						// https://www.andersriggelsen.dk/glblendfunc.php
						// (sR*sA) + (dR*(1-sA)) = rR
						// (sG*sA) + (dG*(1-sA)) = rG
						// (sB*sA) + (dB*(1-sA)) = rB
						// (sA*sA) + (dA*(1-sA)) = rA
						glState.enableFlag(eGlStateFlagType::blend);
						glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
						glBlendEquation(GL_FUNC_ADD);
					}
					break;
			}

			// Apply any Stencils assigned to the node
			if (m_stencilMode != eCompositorStencilMode::noStencil)
			{
				evaluateQuadStencils(glState);
				evaluateBoxStencils(glState);
				evaluateModelStencils(glState);
			}

			// Bind the layer shader program and uniform parameters.
			// This will fail unless all of the shader uniform parameters are bound.
			GlScopedMaterialBinding materialBinding =
				m_material->bindMaterial(
					GlSceneConstPtr(),
					GlCameraConstPtr());

			if (materialBinding)
			{
				assert(m_layerVFlippedMesh);
				assert(m_layerMesh);
				
				if (m_bVerticalFlip)
					m_layerVFlippedMesh->drawElements();
				else
					m_layerMesh->drawElements();
			}
			else
			{
				evaluator.setLastErrorCode(eNodeEvaluationErrorCode::evaluationError);
				evaluator.setLastErrorMessage("Failed to bind material");
				bSuccess = false;
			}
		}
	}

	return bSuccess;
}

FlowPinPtr DrawLayerNode::getOutputFlowPin() const
{
	return getFirstPinOfType<FlowPin>(eNodePinDirection::OUTPUT);
}

void DrawLayerNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// title bar
	if (NodeEditorUI::DrawPropertySheetHeader("Draw Layer Node"))
	{
		// Material
		const std::string material_name = m_material ? m_material->getName() : "<INVALID>";
		NodeEditorUI::DrawStaticTextProperty("Material", material_name);

		// Blend Mode
		int iBlendMode = (int)m_blendMode;
		if (NodeEditorUI::DrawSimpleComboBoxProperty(
			"drawLayerNodeBlendMode",
			"Blend Mode",
			"Blend Off\0Blend On\0",
			iBlendMode))
		{
			m_blendMode= (eCompositorBlendMode)iBlendMode;
		}

		// Stencil Mode
		int iStencilMode = (int)m_stencilMode;
		if (NodeEditorUI::DrawSimpleComboBoxProperty(
			"drawLayerNodeStencilMode",
			"Stencil Mode",
			"None\0Inside\0Outside\0",
			iStencilMode))
		{
			m_stencilMode = (eCompositorStencilMode)iStencilMode;
		}

		// Invert when camera inside stencil options
		NodeEditorUI::DrawCheckBoxProperty(
			"drawLayerNodeNodeStencilInvert",
			"Cam Inside Invert",
			m_bInvertWhenCameraInside);

		// Vertical Flip
		NodeEditorUI::DrawCheckBoxProperty(
			"drawLayerNodeNodeVerticalflip", 
			"Vertical Flip", 
			m_bVerticalFlip);
	}
}

void DrawLayerNode::createLayerQuadMeshes()
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

		m_layerMesh = std::make_shared<GlTriangulatedMesh>("layer_quad_mesh",
														   getLayerQuadVertexDefinition(),
														   (const uint8_t*)x_vertices,
														   4, // 4 verts
														   (const uint8_t*)x_indices,
														   2, // 2 tris
														   false); // mesh doesn't own quad vert data
		m_layerMesh->createBuffers();
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

		m_layerVFlippedMesh = std::make_shared<GlTriangulatedMesh>("layer_vflipped_quad_mesh",
																   getLayerQuadVertexDefinition(),
																   (const uint8_t*)x_vertices,
																   4, // 4 verts
																   (const uint8_t*)x_indices,
																   2, // 2 tris
																   false); // mesh doesn't own quad vert data
		m_layerVFlippedMesh->createBuffers();
	}
}

void DrawLayerNode::createStencilMeshes()
{
	// Create triangulated quad mesh to draw the quad stencils
	{
		static CompositorNodeGraph::StencilVertex x_vertices[] = {
			//   positions
			{glm::vec3(-0.5f,  0.5f, 0.0f)},
			{glm::vec3(-0.5f, -0.5f, 0.0f)},
			{glm::vec3( 0.5f, -0.5f, 0.0f)},
			{glm::vec3( 0.5f,  0.5f, 0.0f)},
		};
		static uint16_t x_indices[] = {0, 1, 2, 0, 2, 3};

		m_stencilQuadMesh = std::make_shared<GlTriangulatedMesh>("quad_stencil_mesh",
														   CompositorNodeGraph::getStencilModelVertexDefinition(),
														   (const uint8_t*)x_vertices,
														   4, // 4 verts in a quad
														   (const uint8_t*)x_indices,
														   2, // 2 tris in a quad
														   false); // mesh doesn't own quad vertex data
		m_stencilQuadMesh->createBuffers();
	}

	// Create triangulated box mesh to draw the box stencils
	{
		static CompositorNodeGraph::StencilVertex x_vertices[] = {
			//   positions
			{glm::vec3(-0.5f,  0.5f, -0.5f)},
			{glm::vec3(-0.5f,  0.5f,  0.5f)},
			{glm::vec3( 0.5f,  0.5f,  0.5f)},
			{glm::vec3( 0.5f,  0.5f, -0.5f)},
			{glm::vec3(-0.5f, -0.5f, -0.5f)},
			{glm::vec3(-0.5f, -0.5f,  0.5f)},
			{glm::vec3( 0.5f, -0.5f,  0.5f)},
			{glm::vec3( 0.5f, -0.5f, -0.5f)},
		};
		static uint16_t x_indices[] = {
			0, 4, 1, 1, 4, 5, // -X Face
			1, 5, 2, 2, 5, 6, // +Z Face
			2, 6, 3, 3, 6, 7, // +X Face
			0, 3, 7, 7, 4, 0, // -Z Face
			5, 4, 6, 6, 4, 7, // -Y Face
			3, 0, 1, 1, 2, 3  // +Y Face 
		};

		m_stencilBoxMesh = std::make_shared<GlTriangulatedMesh>("box_stencil_mesh",
																 CompositorNodeGraph::getStencilModelVertexDefinition(),
																 (const uint8_t*)x_vertices,
																 8, // 8 verts in a cube
																 (const uint8_t*)x_indices,
																 12, // 12 tris in a cube (6 faces * 2 tris/face)
																 false); // mesh doesn't own box vertex data
		m_stencilBoxMesh->createBuffers();
	}
}

void DrawLayerNode::onLinkConnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_materialPin)
	{
		m_materialPin->copyValueFromSourcePin();

		auto materialProperty = std::dynamic_pointer_cast<GraphMaterialProperty>(m_materialPin->getValue());
		if (materialProperty)
		{
			setMaterial(materialProperty->getMaterialResource());

			// Rebuild the pins since the material changed
			rebuildInputPins();
		}
	}
	else if (pin == m_stencilsPin)
	{
		m_stencilsPin->copyValueFromSourcePin();

		rebuildStencilLists();
	}
}

void DrawLayerNode::onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_materialPin)
	{
		setMaterial(GlMaterialPtr());

		// Rebuild the pins since the material changed
		rebuildInputPins();
	}
}

void DrawLayerNode::rebuildInputPins()
{
	// Delete all dynamic material pins
	for (auto it = m_dynamicMaterialPins.begin(); it != m_dynamicMaterialPins.end(); it++)
	{
		NodePinPtr pin = *it;
		
		getOwnerGraph()->deletePinById(pin->getId());
	}
	m_dynamicMaterialPins.clear();

	// Create an input pin for each shader uniform
	if (m_material)
	{
		auto program= m_material->getProgram();

		for (auto it = program->getUniformBegin(); it != program->getUniformEnd(); ++it)
		{
			const std::string& uniformName = it->first;
			eUniformSemantic uniformSemantic = it->second.semantic;
			eUniformDataType uniformDataType = GlProgram::getUniformSemanticDataType(uniformSemantic);
			NodePinPtr newPin;

			switch (uniformDataType)
			{
				case eUniformDataType::datatype_float:
					{
						newPin= addPin<FloatPin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				case eUniformDataType::datatype_float2:
					{
						newPin= addPin<Float2Pin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				case eUniformDataType::datatype_float3:
					{
						newPin= addPin<Float3Pin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				case eUniformDataType::datatype_float4:
					{
						newPin= addPin<Float4Pin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				//TODO
				//case eUniformDataType::datatype_mat4:
				//	{
				//		addPin<Mat4Pin>(uniformName, eNodePinDirection::INPUT);
				//	}
				//	break;
				case eUniformDataType::datatype_texture:
					{
						newPin= addPin<TexturePin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				default:
					assert(false);
			}

			// Keep track of all the dynamic material pins created
			if (newPin)
			{
				m_dynamicMaterialPins.push_back(newPin);
			}
		}
	}
}

const GlVertexDefinition& DrawLayerNode::getLayerQuadVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const int32_t vertexSize = (int32_t)sizeof(DrawLayerNode::QuadVertex);
		std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

		attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position2f, false, vertexSize, offsetof(DrawLayerNode::QuadVertex, aPos)));
		attribs.push_back(GlVertexAttribute(1, eVertexSemantic::texel2f, false, vertexSize, offsetof(DrawLayerNode::QuadVertex, aTexCoords)));

		x_vertexDefinition.vertexSize = vertexSize;
	}

	return x_vertexDefinition;
}

void DrawLayerNode::rebuildStencilLists()
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

			auto stencilProperty= std::static_pointer_cast<GraphStencilProperty>(property);
			StencilComponentPtr stencilComponent= stencilProperty->getStencilComponent();
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

void DrawLayerNode::evaluateQuadStencils(GlState& glState)
{
	EASY_FUNCTION();

	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());
	GlProgramPtr stencilShader = compositorGraph->getStencilShader();

	GlFrameCompositor* frameCompositor= App::getInstance()->getFrameCompositor();
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

	// If the camera is behind all double sided stencil quads
	// reverse the stencil function so that video is drawn inside the stencil.
	// This makes the stencils act like a magic portal into the virtual layers.
	bool bInvertStencils = false;
	if (m_bInvertWhenCameraInside)
	{
		int cameraBehindStencilCount = 0;
		int doubleSidedStencilCount = 0;
		for (QuadStencilComponentPtr stencil : quadStencilList)
		{
			auto stencilConfig = stencil->getQuadStencilDefinition();

			if (!stencilConfig->getIsDisabled() && stencilConfig->getIsDoubleSided())
			{
				const glm::mat4 xform = stencil->getWorldTransform();
				const glm::vec3 quadCenter = glm::vec3(xform[3]);
				const glm::vec3 quadNormal = glm::vec3(xform[2]);
				const glm::vec3 cameraToQuadCenter = quadCenter - cameraPosition;

				if (glm::dot(quadNormal, cameraToQuadCenter) > 0.f)
				{
					cameraBehindStencilCount++;
				}

				doubleSidedStencilCount++;
			}
		}
		bInvertStencils =
			cameraBehindStencilCount > 0 &&
			cameraBehindStencilCount == doubleSidedStencilCount;
	}

	glClearStencil(0);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Do not draw any pixels on the back buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	// Enables testing AND writing functionalities
	glState.enableFlag(eGlStateFlagType::stencilTest);
	// Do not test the current value in the stencil buffer, always accept any value on there for drawing
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	// Make every test succeed
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);


	stencilShader->bindProgram();

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
		stencilShader->setMatrix4x4Uniform(STENCIL_MVP_UNIFORM_NAME, vpMatrix * modelMatrix);

		// Draw the quad
		m_stencilQuadMesh->drawElements();
	}

	stencilShader->unbindProgram();

	// Make sure you will no longer (over)write stencil values, even if any test succeeds
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	// Make sure we draw on the backbuffer again.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	// Now we will only draw pixels where the corresponding stencil buffer value == (nor !=) 1
	GLenum stencilMode = (m_stencilMode == eCompositorStencilMode::outsideStencil) ? GL_NOTEQUAL : GL_EQUAL;
	if (bInvertStencils)
	{
		// Flip the stencil mode from whatever the default was
		stencilMode = (stencilMode == GL_EQUAL) ? GL_NOTEQUAL : GL_EQUAL;
	}
	glStencilFunc(stencilMode, 1, 0xFF);
}

void DrawLayerNode::evaluateBoxStencils(GlState& glState)
{
	EASY_FUNCTION();

	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());
	GlProgramPtr stencilShader= compositorGraph->getStencilShader();

	GlFrameCompositor* frameCompositor= App::getInstance()->getFrameCompositor();
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

	glClearStencil(0);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Do not draw any pixels on the back buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	// Enables testing AND writing functionalities
	glState.enableFlag(eGlStateFlagType::stencilTest);
	// Do not test the current value in the stencil buffer, always accept any value on there for drawing
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	// Make every test succeed
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	stencilShader->bindProgram();

	// Then draw stencil boxes ...
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
		stencilShader->setMatrix4x4Uniform(STENCIL_MVP_UNIFORM_NAME, vpMatrix * modelMatrix);

		// Draw the box
		m_stencilBoxMesh->drawElements();
	}

	stencilShader->unbindProgram();

	// Make sure you will no longer (over)write stencil values, even if any test succeeds
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	// Make sure we draw on the backbuffer again.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	// Now we will only draw pixels where the corresponding stencil buffer value == (nor !=) 1
	const GLenum stencilMode = (m_stencilMode == eCompositorStencilMode::outsideStencil) ? GL_NOTEQUAL : GL_EQUAL;
	glStencilFunc(stencilMode, 1, 0xFF);
}

void DrawLayerNode::evaluateModelStencils(GlState& glState)
{
	EASY_FUNCTION();

	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());
	GlProgramPtr stencilShader= compositorGraph->getStencilShader();

	GlFrameCompositor* frameCompositor = App::getInstance()->getFrameCompositor();
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

	glClearStencil(0);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Do not draw any pixels on the back buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	// Enables testing AND writing functionalities
	glState.enableFlag(eGlStateFlagType::stencilTest);
	// Do not test the current value in the stencil buffer, always accept any value on there for drawing
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	// Make every test succeed
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	// Bind stencil shader
	stencilShader->bindProgram();

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

			// Set the model-view-projection matrix on the stencil shader
			stencilShader->setMatrix4x4Uniform(STENCIL_MVP_UNIFORM_NAME, vpMatrix * modelMatrix);

			for (int meshIndex = 0; meshIndex < (int)renderModelResource->getTriangulatedMeshCount(); ++meshIndex)
			{
				GlTriangulatedMeshPtr mesh = renderModelResource->getTriangulatedMesh(meshIndex);

				mesh->drawElements();
			}
		}
	}

	// Unbind stencil shader
	stencilShader->unbindProgram();

	// Make sure you will no longer (over)write stencil values, even if any test succeeds
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	// Make sure we draw on the backbuffer again.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	// Now we will only draw pixels where the corresponding stencil buffer value == (nor !=) 1
	const GLenum stencilMode = (m_stencilMode == eCompositorStencilMode::outsideStencil) ? GL_NOTEQUAL : GL_EQUAL;
	glStencilFunc(stencilMode, 1, 0xFF);
}

// -- ProgramNode Factory -----
NodePtr DrawLayerNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and default pins
	// The rest of the input pins can't be connected until we have a material assigned
	auto node = std::static_pointer_cast<DrawLayerNode>(NodeFactory::createNode(editorState));

	// Create default input pins
	FlowPinPtr flowInPin = node->addPin<FlowPin>("flowIn", eNodePinDirection::INPUT);

	PropertyPinPtr materialInPin = node->addPin<PropertyPin>("material", eNodePinDirection::INPUT);
	materialInPin->setPropertyClassName(GraphMaterialProperty::k_propertyClassName);
	node->setMaterialPin(materialInPin);

	//TODO: Add vertex definition attribute to the pin
	ArrayPinPtr stencilListInPin = node->addPin<ArrayPin>("stencils", eNodePinDirection::INPUT);
	stencilListInPin->setElementClassName(GraphStencilProperty::k_propertyClassName);
	node->setStencilsPin(stencilListInPin);

	// Create default output pins
	FlowPinPtr flowOutPin = node->addPin<FlowPin>("flowOut", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the default pins to a compatible target pin
	autoConnectInputPin(editorState, flowInPin);
	autoConnectOutputPin(editorState, flowOutPin);
	autoConnectInputPin(editorState, materialInPin);
	autoConnectInputPin(editorState, stencilListInPin);

	return node;
}