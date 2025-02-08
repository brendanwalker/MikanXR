#include "DrawLayerNode.h"	
#include "GlFrameCompositor.h"	
#include "GlMaterial.h"	
#include "MikanRenderModelResource.h"	
#include "MikanModelResourceManager.h"	
#include "IMkShader.h"	
#include "MikanShaderCache.h"	
#include "GlStateStack.h"	
#include "GlStateModifiers.h"	
#include "IMkTexture.h"	
#include "IMkTriangulatedMesh.h"	
#include "GlMaterialInstance.h"	
#include "IMkWindow.h"	
#include "Logger.h"	
#include "MainWindow.h"	
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

	CommonConfig::writeStdMap(pt, "float_defaults", m_floatDefaults);	
	CommonConfig::writeStdArrayMap<float, 2>(pt, "float2_defaults", m_float2Defaults);	
	CommonConfig::writeStdArrayMap<float, 3>(pt, "float3_defaults", m_float3Defaults);	
	CommonConfig::writeStdArrayMap<float, 4>(pt, "float4_defaults", m_float4Defaults);	

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

	CommonConfig::readStdMap(pt, "float_defaults", m_floatDefaults);	
	CommonConfig::readStdArrayMap(pt, "float2_defaults", m_float2Defaults);	
	CommonConfig::readStdArrayMap(pt, "float3_defaults", m_float3Defaults);	
	CommonConfig::readStdArrayMap(pt, "float4_defaults", m_float4Defaults);	
}	

// -- DrawLayerNode -----	
DrawLayerNode::~DrawLayerNode()	
{	
	// Free pin references	
	m_materialPin= nullptr;	
	m_stencilsPin= nullptr;	

	// Clean up render resources	
	m_material = nullptr;	

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
		m_floatDefaults = drawLayerNodeConfig->m_floatDefaults;	
		m_float2Defaults = drawLayerNodeConfig->m_float2Defaults;	
		m_float3Defaults = drawLayerNodeConfig->m_float3Defaults;	
		m_float4Defaults = drawLayerNodeConfig->m_float4Defaults;	

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
	}	

	applyDynamicPinDefaultValues();	
}	

void DrawLayerNode::saveToConfig(NodeConfigPtr nodeConfig) const	
{	
	auto drawLayerNodeConfig = std::static_pointer_cast<DrawLayerNodeConfig>(nodeConfig);	

	for (auto& pin : m_pinsIn)	
	{	
		if (pin->getIsDynamicPin())	
		{	
			if (FloatPinPtr floatPin = std::dynamic_pointer_cast<FloatPin>(pin))	
			{	
				drawLayerNodeConfig->m_floatDefaults[floatPin->getName()] = floatPin->getValue();	
			}	
			else if (Float2PinPtr float2Pin = std::dynamic_pointer_cast<Float2Pin>(pin))	
			{	
				drawLayerNodeConfig->m_float2Defaults[float2Pin->getName()] = float2Pin->getValue();	
			}	
			else if (Float3PinPtr float3Pin = std::dynamic_pointer_cast<Float3Pin>(pin))	
			{	
				drawLayerNodeConfig->m_float3Defaults[float3Pin->getName()] = float3Pin->getValue();	
			}	
			else if (Float4PinPtr float4Pin = std::dynamic_pointer_cast<Float4Pin>(pin))	
			{	
				drawLayerNodeConfig->m_float4Defaults[float4Pin->getName()] = float4Pin->getValue();	
			}	
		}	
	}	

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

void DrawLayerNode::setMaterial(GlMaterialConstPtr inMaterial)	
{	
	m_material = inMaterial;	
	m_materialInstance = 	
		m_material 	
		? std::make_shared<GlMaterialInstance>(inMaterial)	
		: GlMaterialInstancePtr();	
}	

bool DrawLayerNode::evaluateNode(NodeEvaluator& evaluator)	
{	
	bool bSuccess= true;	

	if (!m_material)	
	{	
		evaluator.addError(NodeEvaluationError(eNodeEvaluationErrorCode::missingInput, "Missing material", this));	
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
			// Only consider dynamic pins	
			if (!pin->getIsDynamicPin())	
				continue;	

			if (FloatPinPtr floatPin = std::dynamic_pointer_cast<FloatPin>(pin))	
			{	
				m_materialInstance->setFloatByUniformName(pin->getName(), floatPin->getValue());	
			}	
			else if (Float2PinPtr float2Pin = std::dynamic_pointer_cast<Float2Pin>(pin))	
			{	
				m_materialInstance->setVec2ByUniformName(pin->getName(), glm::make_vec2(float2Pin->getValue().data()));	
			}	
			else if (Float3PinPtr float3Pin = std::dynamic_pointer_cast<Float3Pin>(pin))	
			{	
				m_materialInstance->setVec3ByUniformName(pin->getName(), glm::make_vec3(float3Pin->getValue().data()));	
			}	
			else if (Float4PinPtr float4Pin = std::dynamic_pointer_cast<Float4Pin>(pin))	
			{	
				m_materialInstance->setVec4ByUniformName(pin->getName(), glm::make_vec4(float4Pin->getValue().data()));	
			}	
			// TODO	
			//else if (IntPinPtr floatPin = std::dynamic_pointer_cast<IntPin>(pin))	
			//{	
			//	m_materialInstance->setIntByUniformName(pin->getName(), floatPin->getValue());	
			//}	
			//else if (Int2PinPtr float2Pin = std::dynamic_pointer_cast<Int2Pin>(pin))	
			//{	
			//	m_materialInstance->setInt2ByUniformName(pin->getName(), glm::make_vec2(float2Pin->getValue().data()));	
			//}	
			//else if (Int3PinPtr float3Pin = std::dynamic_pointer_cast<Int3Pin>(pin))	
			//{	
			//	m_materialInstance->setInt3ByUniformName(pin->getName(), glm::make_vec3(float3Pin->getValue().data()));	
			//}	
			//else if (Int4PinPtr float4Pin = std::dynamic_pointer_cast<Int4Pin>(pin))	
			//{	
			//	m_materialInstance->setInt4ByUniformName(pin->getName(), glm::make_vec4(float3Pin->getValue().data()));	
			//}				
			//else if (Mat4PinPtr mat4Pin = std::dynamic_pointer_cast<Mat4Pin>(pin))	
			//{	
			//	m_materialInstance->setMat4ByUniformName(pin->getName(), mat4Pin->getValue()));	
			//}	
			else if (TexturePinPtr texturePin = std::dynamic_pointer_cast<TexturePin>(pin))	
			{	
				GlTexturePtr texturePtr = texturePin->getValue();	
				if (texturePtr)	
				{	
					m_materialInstance->setTextureByUniformName(pin->getName(), texturePtr);	
				}	
			}	
		}	

		{	
			GlScopedState glStateScope = 	
				evaluator.getCurrentWindow()->getGlStateStack().createScopedState("Draw Layer Node");	
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
			GlScopedMaterialBinding materialBinding = m_material->bindMaterial();	
			if (materialBinding)	
			{	
				GlScopedMaterialInstanceBinding materialInstanceBinding = 	
					m_materialInstance->bindMaterialInstance(materialBinding);	

				if (materialInstanceBinding)	
				{	
					auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());	

					if (m_bVerticalFlip)	
						compositorGraph->getLayerVFlippedMesh()->drawElements();	
					else	
						compositorGraph->getLayerMesh()->drawElements();	
				}	
				else	
				{	
					evaluator.addError(	
						NodeEvaluationError(	
							eNodeEvaluationErrorCode::materialError,	
							StringUtils::stringify("Unable to bind ", m_material->getName()),	
							this));	
					for (const auto& iter : materialInstanceBinding.getUnboundUniforms())	
					{	
						evaluator.addError(	
							NodeEvaluationError(	
								eNodeEvaluationErrorCode::materialError,	
								StringUtils::stringify("Missing uniform: ", iter),	
								this));	
					}	
					bSuccess = false;	
				}	
			}	
			else	
			{	
				evaluator.addError(	
					NodeEvaluationError(	
						eNodeEvaluationErrorCode::materialError,	
						StringUtils::stringify("Unable to bind ", m_material->getName()),	
						this));	
				for (const auto& iter : materialBinding.getUnboundUniforms())	
				{	
					evaluator.addError(	
						NodeEvaluationError(	
							eNodeEvaluationErrorCode::materialError, 	
							StringUtils::stringify("Missing uniform: ", iter), 	
							this));	
				}	
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
		setMaterial(GlMaterialConstPtr());	

		// Rebuild the pins since the material changed	
		if (!isPendingDeletion())	
		{	
			rebuildInputPins();	
		}	
	}	
	else if (pin == m_stencilsPin)	
	{	
		m_stencilsPin->clearArray();	

		if (!isPendingDeletion())	
		{	
			rebuildStencilLists();	
		}	
	}	
}	

void DrawLayerNode::rebuildInputPins()	
{	
	assert(!isPendingDeletion());	

	// Delete all dynamic material pins	
	for (auto it = m_pinsIn.begin(); it != m_pinsIn.end(); it++)	
	{	
		NodePinPtr pin = *it;	

		if (pin->getIsDynamicPin())	
		{	
			getOwnerGraph()->deletePinById(pin->getId());	
		}	
	}	

	// Create an input pin for each shader uniform	
	if (m_material)	
	{	
		auto program= m_material->getProgram();	

		for (auto it = program->getUniformBegin(); it != program->getUniformEnd(); ++it)	
		{	
			const std::string& uniformName = it->first;	
			eUniformSemantic uniformSemantic = it->second.semantic;	
			eUniformDataType uniformDataType = getUniformSemanticDataType(uniformSemantic);	
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

			// Flag all new pins as dynamic	
			if (newPin)	
			{	
				newPin->setIsDynamicPin(true);	
			}	
		}	

		// Update dynamic pin default values	
		applyDynamicPinDefaultValues();	
	}	
}	

void DrawLayerNode::applyDynamicPinDefaultValues()	
{	
	for (auto& pin : m_pinsIn)	
	{	
		if (pin->getIsDynamicPin())	
		{	
			const std::string& pinName = pin->getName();	

			if (FloatPinPtr floatPin = std::dynamic_pointer_cast<FloatPin>(pin))	
			{	
				auto defaultIt = m_floatDefaults.find(pinName);	
				if (defaultIt != m_floatDefaults.end())	
				{	
					floatPin->setValue(defaultIt->second);	
				}	
			}	
			else if (Float2PinPtr float2Pin = std::dynamic_pointer_cast<Float2Pin>(pin))	
			{	
				auto defaultIt = m_float2Defaults.find(pinName);	
				if (defaultIt != m_float2Defaults.end())	
				{	
					float2Pin->setValue(defaultIt->second);	
				}	
			}	
			else if (Float3PinPtr float3Pin = std::dynamic_pointer_cast<Float3Pin>(pin))	
			{	
				auto defaultIt = m_float3Defaults.find(pinName);	
				if (defaultIt != m_float3Defaults.end())	
				{	
					float3Pin->setValue(defaultIt->second);	
				}	
			}	
			else if (Float4PinPtr float4Pin = std::dynamic_pointer_cast<Float4Pin>(pin))	
			{	
				auto defaultIt = m_float4Defaults.find(pinName);	
				if (defaultIt != m_float4Defaults.end())	
				{	
					float4Pin->setValue(defaultIt->second);	
				}	
			}	
		}	
	}	
}	

void DrawLayerNode::rebuildStencilLists()	
{	
	assert(!isPendingDeletion());	

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

void DrawLayerNode::evaluateQuadStencils(GlState& glParentState)	
{	
	EASY_FUNCTION();	

	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());	
	IMkTriangulatedMeshPtr stencilQuadMesh = compositorGraph->getStencilQuadMesh();	

	GlFrameCompositor* frameCompositor= MainWindow::getInstance()->getFrameCompositor();	
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

	{	
		GlScopedState stateScope= glParentState.getOwnerStateStack().createScopedState("evaluateQuadStencils");	
		GlState& glState= stateScope.getStackState();	

		glStateSetStencilBufferClearValue(glState, 0);	
		glClear(GL_STENCIL_BUFFER_BIT);	

		// Do not draw any pixels on the back buffer	
		glStateSetColorMask(glState, glm::bvec4(false));	
		glStateSetDepthMask(glState, false);	
		// Enables testing AND writing functionalities	
		glState.enableFlag(eGlStateFlagType::stencilTest);	
		// Do not test the current value in the stencil buffer, always accept any value on there for drawing	
		glStateSetStencilFunc(glState, eGlStencilFunction::ALWAYS, 1, 0xFF);	
		glStateSetStencilMask(glState, 0xFF);	
		// Make every test succeed	
		glStateSetStencilOp(glState, eGlStencilOp::REPLACE, eGlStencilOp::REPLACE, eGlStencilOp::REPLACE);	

		GlMaterialInstancePtr materialInstance = stencilQuadMesh->getMaterialInstance();	
		GlMaterialConstPtr material = materialInstance->getMaterial();	

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
}	

void DrawLayerNode::evaluateBoxStencils(GlState& glParentState)	
{	
	EASY_FUNCTION();	

	auto compositorGraph = std::static_pointer_cast<CompositorNodeGraph>(getOwnerGraph());	
	IMkTriangulatedMeshPtr stencilBoxMesh= compositorGraph->getStencilBoxMesh();	

	GlFrameCompositor* frameCompositor= MainWindow::getInstance()->getFrameCompositor();	
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

	{	
		GlScopedState stateScope = glParentState.getOwnerStateStack().createScopedState("evaluateBoxStencils");	
		GlState& glState = stateScope.getStackState();	

		glStateSetStencilBufferClearValue(glState, 0);	
		glClear(GL_STENCIL_BUFFER_BIT);	

		// Do not draw any pixels on the back buffer	
		glStateSetColorMask(glState, glm::bvec4(false));	
		glStateSetDepthMask(glState, false);	
		// Enables testing AND writing functionalities	
		glState.enableFlag(eGlStateFlagType::stencilTest);	
		// Do not test the current value in the stencil buffer, always accept any value on there for drawing	
		glStateSetStencilFunc(glState, eGlStencilFunction::ALWAYS, 1, 0xFF);	
		glStateSetStencilMask(glState, 0xFF);	
		// Make every test succeed	
		glStateSetStencilOp(glState, eGlStencilOp::REPLACE, eGlStencilOp::REPLACE, eGlStencilOp::REPLACE);	

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
}	

void DrawLayerNode::evaluateModelStencils(GlState& glParentState)	
{	
	EASY_FUNCTION();	

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

	{	
		GlScopedState stateScope = glParentState.getOwnerStateStack().createScopedState("evaluateModelStencils");	
		GlState& glState = stateScope.getStackState();	

		glStateSetStencilBufferClearValue(glState, 0);	
		glClear(GL_STENCIL_BUFFER_BIT);	

		// Do not draw any pixels on the back buffer	
		glStateSetColorMask(glState, glm::bvec4(false));	
		glStateSetDepthMask(glState, false);	
		// Enables testing AND writing functionalities	
		glState.enableFlag(eGlStateFlagType::stencilTest);	
		// Do not test the current value in the stencil buffer, always accept any value on there for drawing	
		glStateSetStencilFunc(glState, eGlStencilFunction::ALWAYS, 1, 0xFF);	
		glStateSetStencilMask(glState, 0xFF);	
		// Make every test succeed	
		glStateSetStencilOp(glState, eGlStencilOp::REPLACE, eGlStencilOp::REPLACE, eGlStencilOp::REPLACE);	


		// Then draw stencil models	
		for (ModelStencilComponentPtr stencil : modelStencilList)	
		{	
			auto stencilConfig = stencil->getModelStencilDefinition();	
			const MikanStencilID stencilId = stencilConfig->getStencilId();	
			MikanRenderModelResourcePtr renderModelResource =	
				compositorGraph->getOrLoadStencilRenderModel(stencilConfig);	

			if (renderModelResource)	
			{	
				// Set the model matrix of stencil model	
				const glm::mat4 modelMatrix = stencil->getWorldTransform();	
				const glm::mat4 mvpMatrix = vpMatrix * modelMatrix;	

				for (int meshIndex = 0; meshIndex < renderModelResource->getTriangulatedMeshCount(); ++meshIndex)	
				{	
					IMkTriangulatedMeshPtr mesh = renderModelResource->getTriangulatedMesh(meshIndex);	
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
	stencilListInPin->setHasDefaultValue(true); // Unconnected array pin == no stencils	
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