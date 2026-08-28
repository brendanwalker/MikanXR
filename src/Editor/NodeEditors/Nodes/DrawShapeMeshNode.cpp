#include "DrawShapeMeshNode.h"
#include "IconsForkAwesome.h"
#include "BoxShapeComponent.h"
#include "CompositorConstants.h"
#include "IMkGraphicsContext.h"
#include "IMkSceneRenderable.h"
#include "IMkStaticMeshInstance.h"
#include "IMkTexture.h"
#include "IMkMesh.h"
#include "IMkShader.h"
#include "LocText.h"
#include "Logger.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "IMkState.h"
#include "ModelShapeComponent.h"
#include "NodeEditorState.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "QuadShapeComponent.h"
#include "ShapeComponent.h"
#include "StaticMeshComponent.h"
#include "StringUtils.h"

#include "Graphs/ShapeNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include "Pins/FloatPin.h"
#include "Pins/FlowPin.h"
#include "Pins/NodePin.h"
#include "Pins/PropertyPin.h"
#include "Pins/TexturePin.h"

#include "Properties/GraphMaterialProperty.h"

#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

// -- DrawShapeMeshNodeConfig -----
configuru::Config DrawShapeMeshNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["blend_mode"]= k_compositorBlendModeStrings[(int)blendMode];
	pt["depth_test"]= bDepthTest;

	CommonConfig::writeStdMap(pt, "float_defaults", m_floatDefaults);
	CommonConfig::writeStdArrayMap<float, 2>(pt, "float2_defaults", m_float2Defaults);
	CommonConfig::writeStdArrayMap<float, 3>(pt, "float3_defaults", m_float3Defaults);
	CommonConfig::writeStdArrayMap<float, 4>(pt, "float4_defaults", m_float4Defaults);

	return pt;
}

void DrawShapeMeshNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	std::string blendModeString=
		pt.get_or<std::string>("blend_mode", k_compositorBlendModeStrings[(int)eCompositorBlendMode::blendNormal]);
	blendMode= StringUtils::FindEnumValue<eCompositorBlendMode>(blendModeString, k_compositorBlendModeStrings);

	bDepthTest= pt.get_or<bool>("depth_test", false);

	CommonConfig::readStdMap(pt, "float_defaults", m_floatDefaults);
	CommonConfig::readStdArrayMap(pt, "float2_defaults", m_float2Defaults);
	CommonConfig::readStdArrayMap(pt, "float3_defaults", m_float3Defaults);
	CommonConfig::readStdArrayMap(pt, "float4_defaults", m_float4Defaults);
}

// -- DrawShapeMeshNode -----
bool DrawShapeMeshNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const DrawShapeMeshNodeConfig>(nodeConfig);

		m_blendMode= config->blendMode;
		m_bDepthTest= config->bDepthTest;
		m_floatDefaults= config->m_floatDefaults;
		m_float2Defaults= config->m_float2Defaults;
		m_float3Defaults= config->m_float3Defaults;
		m_float4Defaults= config->m_float4Defaults;

		return true;
	}

	return false;
}

void DrawShapeMeshNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<DrawShapeMeshNodeConfig>(nodeConfig);

	for (auto& pin : m_pinsIn)
	{
		if (pin->getIsDynamicPin())
		{
			if (FloatPinPtr floatPin= std::dynamic_pointer_cast<FloatPin>(pin))
			{
				config->m_floatDefaults[floatPin->getName()]= floatPin->getValue();
			}
			else if (Float2PinPtr float2Pin= std::dynamic_pointer_cast<Float2Pin>(pin))
			{
				config->m_float2Defaults[float2Pin->getName()]= float2Pin->getValue();
			}
			else if (Float3PinPtr float3Pin= std::dynamic_pointer_cast<Float3Pin>(pin))
			{
				config->m_float3Defaults[float3Pin->getName()]= float3Pin->getValue();
			}
			else if (Float4PinPtr float4Pin= std::dynamic_pointer_cast<Float4Pin>(pin))
			{
				config->m_float4Defaults[float4Pin->getName()]= float4Pin->getValue();
			}
		}
	}

	config->blendMode= m_blendMode;
	config->bDepthTest= m_bDepthTest;

	Node::saveToConfig(nodeConfig);
}

void DrawShapeMeshNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnGraphLoaded-= MakeDelegate(this, &DrawShapeMeshNode::onGraphLoaded);
			m_ownerGraph= nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnGraphLoaded+= MakeDelegate(this, &DrawShapeMeshNode::onGraphLoaded);
			m_ownerGraph= newOwnerGraph;
		}
	}
}

void DrawShapeMeshNode::setMaterialPin(PropertyPinPtr inPin) { m_materialPin= inPin; }

void DrawShapeMeshNode::setMaterial(MkMaterialConstPtr inMaterial)
{
	m_material= inMaterial;
	m_materialInstance= m_material ? createMkMaterialInstance(inMaterial) : MkMaterialInstancePtr();
}

void DrawShapeMeshNode::onGraphLoaded(bool success)
{
	if (success)
	{
		// Make sure we have a material input pin
		PropertyPinPtr materialInPin= getFirstPinOfType<PropertyPin>(eNodePinDirection::INPUT);
		if (materialInPin && materialInPin->getPropertyClassName() == GraphMaterialProperty::k_propertyClassName)
		{
			setMaterialPin(materialInPin);
			m_materialPin->copyValueFromSourcePin();

			auto materialProperty= std::dynamic_pointer_cast<GraphMaterialProperty>(m_materialPin->getValue());
			if (materialProperty)
			{
				setMaterial(materialProperty->getMaterialResource());
			}
		}
	}

	applyDynamicPinDefaultValues();
}

void DrawShapeMeshNode::onLinkConnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_materialPin)
	{
		m_materialPin->copyValueFromSourcePin();

		auto materialProperty= std::dynamic_pointer_cast<GraphMaterialProperty>(m_materialPin->getValue());
		if (materialProperty)
		{
			setMaterial(materialProperty->getMaterialResource());

			// Rebuild the pins since the material changed
			rebuildInputPins();
		}
	}
}

void DrawShapeMeshNode::onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_materialPin)
	{
		setMaterial(MkMaterialConstPtr());

		// Rebuild the pins since the material changed
		if (!isPendingDeletion())
		{
			rebuildInputPins();
		}
	}
}

void DrawShapeMeshNode::rebuildInputPins()
{
	assert(!isPendingDeletion());

	// Delete all dynamic material pins
	for (int pinIndex= (int)m_pinsIn.size() - 1; pinIndex >= 0; pinIndex--)
	{
		NodePinPtr pin= m_pinsIn[pinIndex];

		if (pin->getIsDynamicPin())
		{
			getOwnerGraph()->deletePinById(pin->getId());
		}
	}

	// Create an input pin for each shader uniform
	if (m_material)
	{
		auto program= m_material->getProgram();

		for (auto it= program->getUniformBegin(); it != program->getUniformEnd(); ++it)
		{
			const std::string& uniformName= it->first;
			eUniformSemantic uniformSemantic= it->second.semantic;
			eUniformDataType uniformDataType= getUniformSemanticDataType(uniformSemantic);
			NodePinPtr newPin;

			switch (uniformDataType)
			{
			case eUniformDataType::datatype_float:
				newPin= addPin<FloatPin>(uniformName, eNodePinDirection::INPUT);
				break;
			case eUniformDataType::datatype_float2:
				newPin= addPin<Float2Pin>(uniformName, eNodePinDirection::INPUT);
				break;
			case eUniformDataType::datatype_float3:
				newPin= addPin<Float3Pin>(uniformName, eNodePinDirection::INPUT);
				break;
			case eUniformDataType::datatype_float4:
				newPin= addPin<Float4Pin>(uniformName, eNodePinDirection::INPUT);
				break;
			case eUniformDataType::datatype_texture:
				newPin= addPin<TexturePin>(uniformName, eNodePinDirection::INPUT);
				break;
			case eUniformDataType::datatype_mat4:
				if (uniformSemantic == eUniformSemantic::modelViewProjectionMatrix)
				{
					// The node graph will automatically feed in the correct value for this semantic, so we don't need
					// an input pin for it
					break;
				}
				else
				{
					MIKAN_LOG_WARNING("DrawShapeMeshNode::rebuildInputPins")
						<< "DrawShapeMeshNode does not support mat4 uniform" << uniformName;
					assert(false);
				}
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

void DrawShapeMeshNode::applyDynamicPinDefaultValues()
{
	for (auto& pin : m_pinsIn)
	{
		if (pin->getIsDynamicPin())
		{
			const std::string& pinName= pin->getName();

			if (FloatPinPtr floatPin= std::dynamic_pointer_cast<FloatPin>(pin))
			{
				auto defaultIt= m_floatDefaults.find(pinName);
				if (defaultIt != m_floatDefaults.end())
					floatPin->setValue(defaultIt->second);
			}
			else if (Float2PinPtr float2Pin= std::dynamic_pointer_cast<Float2Pin>(pin))
			{
				auto defaultIt= m_float2Defaults.find(pinName);
				if (defaultIt != m_float2Defaults.end())
					float2Pin->setValue(defaultIt->second);
			}
			else if (Float3PinPtr float3Pin= std::dynamic_pointer_cast<Float3Pin>(pin))
			{
				auto defaultIt= m_float3Defaults.find(pinName);
				if (defaultIt != m_float3Defaults.end())
					float3Pin->setValue(defaultIt->second);
			}
			else if (Float4PinPtr float4Pin= std::dynamic_pointer_cast<Float4Pin>(pin))
			{
				auto defaultIt= m_float4Defaults.find(pinName);
				if (defaultIt != m_float4Defaults.end())
					float4Pin->setValue(defaultIt->second);
			}
		}
	}
}

bool DrawShapeMeshNode::evaluateNode(NodeEvaluator& evaluator)
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

	if (!bSuccess)
		return false;

	auto shapeGraph= std::static_pointer_cast<ShapeNodeGraph>(getOwnerGraph());
	ShapeComponentPtr shape= shapeGraph->getBoundShapeComponent();
	if (!shape)
	{
		evaluator.addError(
			NodeEvaluationError(eNodeEvaluationErrorCode::missingInput, "No bound ShapeComponent", this));
		return false;
	}

	const glm::mat4& vpMatrix= shapeGraph->getRenderVPMatrix();

	// Map dynamic input pins onto the material instance
	for (auto& pin : m_pinsIn)
	{
		if (!pin->getIsDynamicPin())
			continue;

		if (FloatPinPtr floatPin= std::dynamic_pointer_cast<FloatPin>(pin))
		{
			m_materialInstance->setFloatByUniformName(pin->getName(), floatPin->getValue());
		}
		else if (Float2PinPtr float2Pin= std::dynamic_pointer_cast<Float2Pin>(pin))
		{
			m_materialInstance->setVec2ByUniformName(pin->getName(), glm::make_vec2(float2Pin->getValue().data()));
		}
		else if (Float3PinPtr float3Pin= std::dynamic_pointer_cast<Float3Pin>(pin))
		{
			m_materialInstance->setVec3ByUniformName(pin->getName(), glm::make_vec3(float3Pin->getValue().data()));
		}
		else if (Float4PinPtr float4Pin= std::dynamic_pointer_cast<Float4Pin>(pin))
		{
			m_materialInstance->setVec4ByUniformName(pin->getName(), glm::make_vec4(float4Pin->getValue().data()));
		}
		else if (TexturePinPtr texturePin= std::dynamic_pointer_cast<TexturePin>(pin))
		{
			IMkTexturePtr texturePtr= texturePin->getValue();
			if (texturePtr)
			{
				m_materialInstance->setTextureByUniformName(pin->getName(), texturePtr);
			}
		}
	}

	IMkGraphicsContext* graphicsContext= evaluator.getCurrentGraphicsContext();
	MkScopedState mkStateScope= graphicsContext->getMkStateStack().createScopedState("Draw Shape Mesh Node");
	IMkState* mkState= mkStateScope.getStackState();

	// Set blend mode
	switch (m_blendMode)
	{
	case eCompositorBlendMode::blendOff:
		mkState->disableFlag(eMkStateFlagType::blend);
		break;
	case eCompositorBlendMode::blendNormal:
		mkState->enableFlag(eMkStateFlagType::blend);
		mkStateSetBlendFunc(mkState, eMkBlendFunction::SRC_ALPHA, eMkBlendFunction::ONE_MINUS_SRC_ALPHA);
		mkStateSetBlendEquation(mkState, eMkBlendEquation::ADD);
		break;
	case eCompositorBlendMode::blendMultiply:
		mkState->enableFlag(eMkStateFlagType::blend);
		mkStateSetBlendFunc(mkState, eMkBlendFunction::DST_COLOR, eMkBlendFunction::ZERO);
		mkStateSetBlendEquation(mkState, eMkBlendEquation::ADD);
		break;
	}

	// Depth test
	if (m_bDepthTest)
		mkState->enableFlag(eMkStateFlagType::depthTest);
	else
		mkState->disableFlag(eMkStateFlagType::depthTest);

	// Render all renderables from the bound shape component
	if (auto quadShape= std::dynamic_pointer_cast<QuadShapeComponent>(shape))
	{
		IMkSceneRenderableConstPtr renderable= quadShape->getGlSceneRenderableConst();
		if (renderable)
			drawShapeRenderable(renderable, vpMatrix);
	}
	else if (auto boxShape= std::dynamic_pointer_cast<BoxShapeComponent>(shape))
	{
		IMkSceneRenderableConstPtr renderable= boxShape->getGlSceneRenderableConst();
		if (renderable)
			drawShapeRenderable(renderable, vpMatrix);
	}
	else if (auto modelShape= std::dynamic_pointer_cast<ModelShapeComponent>(shape))
	{
		for (auto& meshComp : modelShape->getTriangulatedMeshes())
		{
			IMkSceneRenderableConstPtr renderable= meshComp->getGlSceneRenderableConst();
			if (renderable)
				drawShapeRenderable(renderable, vpMatrix);
		}
	}

	return true;
}

void DrawShapeMeshNode::drawShapeRenderable(IMkSceneRenderableConstPtr renderable, const glm::mat4& vpMatrix)
{
	// Compute MVP without storing it on the material instance
	const glm::mat4 mvpMatrix= vpMatrix * renderable->getModelMatrix();

	// Bind material and draw, injecting MVP transiently via callback so it is
	// never written into mat4Sources on the shared material instance
	if (auto materialBinding= m_material->bindMaterial())
	{
		BindUniformCallback mvpCallback= [&mvpMatrix](IMkShaderPtr program, eUniformDataType dataType,
													  eUniformSemantic semantic,
													  const std::string& uniformName) -> eUniformBindResult
		{
			if (semantic == eUniformSemantic::modelViewProjectionMatrix)
			{
				return program->setMatrix4x4Uniform(uniformName, mvpMatrix) ? eUniformBindResult::bound
																			: eUniformBindResult::error;
			}
			return eUniformBindResult::unbound;
		};

		if (auto matInstBinding= m_materialInstance->bindMaterialInstance(materialBinding, mvpCallback))
		{
			auto staticMeshInst= std::dynamic_pointer_cast<const IMkStaticMeshInstance>(renderable);
			if (staticMeshInst)
			{
				IMkMeshConstPtr mesh= staticMeshInst->getMesh();
				if (mesh)
					mesh->drawElements();
			}
		}
		else
		{
			// Material binding failed — log unbound uniforms
		}
	}
}

FlowPinPtr DrawShapeMeshNode::getOutputFlowPin() const { return getFirstPinOfType<FlowPin>(eNodePinDirection::OUTPUT); }

void DrawShapeMeshNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.drawShapeMeshHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		// Material
		const std::string materialName= m_material ? m_material->getName() : "<INVALID>";
		MkGui::drawStaticTextProperty(propertyStyle, locText("nodes.material"), materialName);

		// Blend Mode
		const std::string blendModeItems=
			std::string(locText("nodes.blendOff")) + '\0' + locText("nodes.blendOn") + '\0';
		int iBlendMode= (int)m_blendMode;
		if (MkGui::drawSimpleComboBoxProperty(propertyStyle, "drawShapeMeshNodeBlendMode", locText("nodes.blendMode"),
											  blendModeItems.c_str(), iBlendMode))
		{
			m_blendMode= (eCompositorBlendMode)iBlendMode;
		}

		// Depth Test
		MkGui::drawCheckBoxProperty(propertyStyle, "drawShapeMeshNodeDepthTest", locText("nodes.depthTest"),
									m_bDepthTest);
	}
}

// -- DrawShapeMeshNodeFactory -----
NodePtr DrawShapeMeshNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<DrawShapeMeshNode>(NodeFactory::createNode(editorState));

	// Flow in
	FlowPinPtr flowInPin= node->addPin<FlowPin>("flowIn", eNodePinDirection::INPUT);

	// Material property pin
	PropertyPinPtr materialInPin= node->addPin<PropertyPin>("material", eNodePinDirection::INPUT);
	materialInPin->setPropertyClassName(GraphMaterialProperty::k_propertyClassName);
	node->setMaterialPin(materialInPin);

	// Flow out
	FlowPinPtr flowOutPin= node->addPin<FlowPin>("flowOut", eNodePinDirection::OUTPUT);

	autoConnectInputPin(editorState, flowInPin);
	autoConnectOutputPin(editorState, flowOutPin);
	autoConnectInputPin(editorState, materialInPin);

	return node;
}

const char* DrawShapeMeshNode::editorGetHeaderIcon() const { return ICON_FK_CUBES; }
