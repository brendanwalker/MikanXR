#include "DrawShapeMeshNode.h"
#include "IconsForkAwesome.h"
#include "BoxShapeComponent.h"
#include "IMkGraphicsContext.h"
#include "IMkSceneRenderable.h"
#include "IMkStaticMeshInstance.h"
#include "IMkTexture.h"
#include "IMkMesh.h"
#include "IMkShader.h"
#include "IMkVertexDefinition.h"
#include "LocText.h"
#include "Logger.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
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

#include <algorithm>

// -- DrawShapeMeshNodeConfig -----
configuru::Config DrawShapeMeshNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	CommonConfig::writeStdMap(pt, "float_defaults", m_floatDefaults);
	CommonConfig::writeStdArrayMap<float, 2>(pt, "float2_defaults", m_float2Defaults);
	CommonConfig::writeStdArrayMap<float, 3>(pt, "float3_defaults", m_float3Defaults);
	CommonConfig::writeStdArrayMap<float, 4>(pt, "float4_defaults", m_float4Defaults);

	return pt;
}

void DrawShapeMeshNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	CommonConfig::readStdMap(pt, "float_defaults", m_floatDefaults);
	CommonConfig::readStdArrayMap(pt, "float2_defaults", m_float2Defaults);
	CommonConfig::readStdArrayMap(pt, "float3_defaults", m_float3Defaults);
	CommonConfig::readStdArrayMap(pt, "float4_defaults", m_float4Defaults);
}

// -- DrawShapeMeshNode -----
DrawShapeMeshNode::~DrawShapeMeshNode()
{
	// Free pin references
	m_materialPin= nullptr;

	// Clean up render resources
	m_materialInstance= nullptr;
	m_material= nullptr;

	// Stop listening to events from owner graph
	setOwnerGraph(NodeGraphPtr());
}

bool DrawShapeMeshNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const DrawShapeMeshNodeConfig>(nodeConfig);

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

	Node::saveToConfig(nodeConfig);
}

void DrawShapeMeshNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnGraphLoaded-= MakeDelegate(this, &DrawShapeMeshNode::onGraphLoaded);
			m_ownerGraph->OnPropertyModifed-= MakeDelegate(this, &DrawShapeMeshNode::onGraphPropertyModified);
			m_ownerGraph= nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnGraphLoaded+= MakeDelegate(this, &DrawShapeMeshNode::onGraphLoaded);
			newOwnerGraph->OnPropertyModifed+= MakeDelegate(this, &DrawShapeMeshNode::onGraphPropertyModified);
			m_ownerGraph= newOwnerGraph;
		}
	}
}

void DrawShapeMeshNode::onGraphPropertyModified(t_graph_property_id id)
{
	if (!m_materialPin || isPendingDeletion())
		return;

	auto materialProperty= std::dynamic_pointer_cast<GraphMaterialProperty>(m_materialPin->getValue());
	if (materialProperty && materialProperty->getId() == id)
	{
		// Recreate the instance against the recompiled program and track its uniforms
		setMaterial(materialProperty->getMaterialResource());
		rebuildInputPins();
	}
}

bool DrawShapeMeshNode::editorCanAcceptProperty(NodePinPtr pin, GraphPropertyPtr property) const
{
	if (pin != m_materialPin)
		return true;

	auto materialProperty= std::dynamic_pointer_cast<GraphMaterialProperty>(property);
	return !materialProperty || materialProperty->isCompatibleWithDomain(eMaterialDomain::shape);
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
				// Loaded pins carry the flags of the material as it was saved, so refresh them against
				// the material as loaded now (a default texture makes its pin optional)
				rebuildInputPins();
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

	// Index the dynamic pins by name. A pin whose uniform survives with the same
	// data type is kept, so its links and value carry across a material reload.
	captureDynamicPinDefaultValues();
	std::map<std::string, NodePinPtr> unclaimedPins;
	for (NodePinPtr pin : m_pinsIn)
	{
		if (pin->getIsDynamicPin())
		{
			unclaimedPins.insert({pin->getName(), pin});
		}
	}

	// Claim or create an input pin for each shader uniform, in uniform order
	std::vector<NodePinPtr> dynamicPins;
	IMkShaderPtr program= m_material ? m_material->getProgram() : IMkShaderPtr();
	if (program)
	{
		for (auto it= program->getUniformBegin(); it != program->getUniformEnd(); ++it)
		{
			const std::string& uniformName= it->first;
			const eUniformSemantic uniformSemantic= it->second.semantic;
			const eUniformDataType uniformDataType= getUniformSemanticDataType(uniformSemantic);

			if (uniformDataType == eUniformDataType::datatype_mat4)
			{
				// The node feeds the model-view-projection itself, so it gets no pin
				if (uniformSemantic != eUniformSemantic::modelViewProjectionMatrix)
				{
					MIKAN_LOG_WARNING("DrawShapeMeshNode::rebuildInputPins")
						<< "DrawShapeMeshNode does not support mat4 uniform" << uniformName;
					assert(false);
				}
				continue;
			}

			const std::string& pinClassName= GraphMaterialProperty::getUniformPinClassName(uniformDataType);
			if (pinClassName.empty())
			{
				assert(false);
				continue;
			}

			NodePinPtr pin;
			auto unclaimedIt= unclaimedPins.find(uniformName);
			if (unclaimedIt != unclaimedPins.end() && unclaimedIt->second->getClassName() == pinClassName)
			{
				pin= unclaimedIt->second;
				unclaimedPins.erase(unclaimedIt);
			}
			else
			{
				pin= addPinByClassName(pinClassName, uniformName, eNodePinDirection::INPUT);
				pin->setIsDynamicPin(true);
				GraphMaterialProperty::initDynamicPinFromMaterialDefault(pin, m_material);
			}

			// A texture the material carries a default for need not be connected
			if (uniformDataType == eUniformDataType::datatype_texture)
			{
				IMkTextureConstPtr defaultTexture;
				pin->setHasDefaultValue(m_material->getTextureByUniformName(uniformName, defaultTexture)
										&& defaultTexture != nullptr);
			}

			dynamicPins.push_back(pin);
		}
	}

	// Delete the dynamic pins no uniform claimed
	for (const auto& [pinName, unclaimedPin] : unclaimedPins)
	{
		getOwnerGraph()->deletePinById(unclaimedPin->getId());
	}

	// Keep the dynamic pins after the fixed ones, in uniform order
	m_pinsIn.erase(
		std::remove_if(m_pinsIn.begin(), m_pinsIn.end(), [](const NodePinPtr& pin) { return pin->getIsDynamicPin(); }),
		m_pinsIn.end());
	m_pinsIn.insert(m_pinsIn.end(), dynamicPins.begin(), dynamicPins.end());

	// Update dynamic pin default values
	applyDynamicPinDefaultValues();
}

void DrawShapeMeshNode::captureDynamicPinDefaultValues()
{
	for (auto& pin : m_pinsIn)
	{
		if (!pin->getIsDynamicPin())
			continue;

		if (FloatPinPtr floatPin= std::dynamic_pointer_cast<FloatPin>(pin))
		{
			m_floatDefaults[floatPin->getName()]= floatPin->getValue();
		}
		else if (Float2PinPtr float2Pin= std::dynamic_pointer_cast<Float2Pin>(pin))
		{
			m_float2Defaults[float2Pin->getName()]= float2Pin->getValue();
		}
		else if (Float3PinPtr float3Pin= std::dynamic_pointer_cast<Float3Pin>(pin))
		{
			m_float3Defaults[float3Pin->getName()]= float3Pin->getValue();
		}
		else if (Float4PinPtr float4Pin= std::dynamic_pointer_cast<Float4Pin>(pin))
		{
			m_float4Defaults[float4Pin->getName()]= float4Pin->getValue();
		}
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

	// Blend and depth state come from the caller: DrawShapesNode's blend mode and depth test in the
	// compositor, the depth tested scene pass in the project view. The graph only picks the material.

	// Collect the renderables of the bound shape component
	std::vector<IMkSceneRenderableConstPtr> renderables;
	if (auto quadShape= std::dynamic_pointer_cast<QuadShapeComponent>(shape))
	{
		IMkSceneRenderableConstPtr renderable= quadShape->getGlSceneRenderableConst();
		if (renderable)
			renderables.push_back(renderable);
	}
	else if (auto boxShape= std::dynamic_pointer_cast<BoxShapeComponent>(shape))
	{
		IMkSceneRenderableConstPtr renderable= boxShape->getGlSceneRenderableConst();
		if (renderable)
			renderables.push_back(renderable);
	}
	else if (auto modelShape= std::dynamic_pointer_cast<ModelShapeComponent>(shape))
	{
		for (auto& meshComp : modelShape->getTriangulatedMeshes())
		{
			IMkSceneRenderableConstPtr renderable= meshComp->getGlSceneRenderableConst();
			if (renderable)
				renderables.push_back(renderable);
		}
	}

	return drawShapeRenderables(evaluator, renderables, vpMatrix);
}

bool DrawShapeMeshNode::drawShapeRenderables(NodeEvaluator& evaluator,
											 const std::vector<IMkSceneRenderableConstPtr>& renderables,
											 const glm::mat4& vpMatrix)
{
	IMkShaderPtr materialProgram= m_material->getProgram();
	if (!materialProgram)
	{
		evaluator.addError(NodeEvaluationError(eNodeEvaluationErrorCode::materialError,
											   StringUtils::stringify("No program on ", m_material->getName()), this));
		return false;
	}

	IMkVertexDefinitionConstPtr materialLayout= materialProgram->getVertexDefinition();
	bool bLayoutMismatchReported= false;

	for (IMkSceneRenderableConstPtr renderable : renderables)
	{
		auto staticMeshInst= std::dynamic_pointer_cast<const IMkStaticMeshInstance>(renderable);
		IMkMeshConstPtr mesh= staticMeshInst ? staticMeshInst->getMesh() : IMkMeshConstPtr();
		if (!mesh)
			continue;

		// A mesh's vertex buffer is laid out by the material it was built with. A
		// program expecting another layout would read the attributes as garbage.
		MkMaterialInstancePtr meshMaterialInstance= mesh->getMaterialInstance();
		MkMaterialConstPtr meshMaterial=
			meshMaterialInstance ? meshMaterialInstance->getMaterial() : MkMaterialConstPtr();
		IMkShaderPtr meshProgram= meshMaterial ? meshMaterial->getProgram() : IMkShaderPtr();
		IMkVertexDefinitionConstPtr meshLayout=
			meshProgram ? meshProgram->getVertexDefinition() : IMkVertexDefinitionConstPtr();
		if (meshLayout && !materialLayout->isCompatibleDefinition(meshLayout))
		{
			if (!bLayoutMismatchReported)
			{
				evaluator.addError(NodeEvaluationError(
					eNodeEvaluationErrorCode::materialError,
					StringUtils::stringify("Material vertex layout ", materialLayout->getVertexDefinitionDesc(),
										   " does not match mesh ", meshLayout->getVertexDefinitionDesc()),
					this));
				bLayoutMismatchReported= true;
			}
			continue;
		}

		// Compute MVP without storing it on the material instance
		drawMesh(mesh, vpMatrix * renderable->getModelMatrix());
	}

	return !bLayoutMismatchReported;
}

void DrawShapeMeshNode::drawMesh(IMkMeshConstPtr mesh, const glm::mat4& mvpMatrix)
{
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
			mesh->drawElements();
		}
	}
}

FlowPinPtr DrawShapeMeshNode::getOutputFlowPin() const { return getFirstPinOfType<FlowPin>(eNodePinDirection::OUTPUT); }

void DrawShapeMeshNode::editorOnDoubleClicked(const NodeEditorState& editorState)
{
	auto materialProperty= m_materialPin ? std::dynamic_pointer_cast<GraphMaterialProperty>(m_materialPin->getValue())
										 : GraphMaterialPropertyPtr();
	if (materialProperty)
	{
		materialProperty->editorOpenSourceGraph();
	}
}

void DrawShapeMeshNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.drawShapeMeshHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		// Material
		const std::string materialName= m_material ? m_material->getName() : "<INVALID>";
		MkGui::drawStaticTextProperty(propertyStyle, locText("nodes.material"), materialName);
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
