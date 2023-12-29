#include "DrawLayerNode.h"
#include "GlMaterial.h"
#include "GlRenderModelResource.h"
#include "GlProgram.h"
#include "GlStateStack.h"
#include "GlTexture.h"
#include "GlTriangulatedMesh.h"
#include "IGlWindow.h"
#include "EditorNodeUtil.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"
#include "Graphs/NodeGraph.h"
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

// -- DrawLayerNodeConfig -----
configuru::Config DrawLayerNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["blend_mode"] = k_compositorBlendModeStrings[(int)blendMode];
	pt["vertical_flip"] = bVerticalFlip;

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

	bVerticalFlip = pt.get_or<bool>("vertical_flip", false);
}

// -- DrawLayerNode -----
DrawLayerNode::~DrawLayerNode()
{
	// Clean up the triangulated mesh resources
	m_layerVFlippedQuad= nullptr;
	m_layerMesh= nullptr;

	setOwnerGraph(NodeGraphPtr());
	setMaterialPin(PropertyPinPtr());
}

bool DrawLayerNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto drawLayerNodeConfig = std::static_pointer_cast<const DrawLayerNodeConfig>(nodeConfig);

		m_blendMode= drawLayerNodeConfig->blendMode;
		m_bVerticalFlip = drawLayerNodeConfig->bVerticalFlip;

		return true;
	}

	return false;
}

void DrawLayerNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto drawLayerNodeConfig = std::static_pointer_cast<DrawLayerNodeConfig>(nodeConfig);

	drawLayerNodeConfig->blendMode = m_blendMode;
	drawLayerNodeConfig->bVerticalFlip = m_bVerticalFlip;

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
	if (inPin != m_materialPin)
	{
		if (m_materialPin)
		{
			m_materialPin = nullptr;
		}

		if (inPin)
		{
			m_materialPin = inPin;
		}
	}
}

void DrawLayerNode::setMaterial(GlMaterialPtr inMaterial)
{
	if (inMaterial != m_material)
	{
		m_material = inMaterial;

		// Rebuild the pins since the material changed
		rebuildInputPins();
	}
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

			// Bind the layer shader program and uniform parameters.
			// This will fail unless all of the shader uniform parameters are bound.
			GlScopedMaterialBinding materialBinding =
				m_material->bindMaterial(
					GlSceneConstPtr(),
					GlCameraConstPtr());

			if (materialBinding)
			{
				assert(m_layerVFlippedQuad);
				assert(m_layerMesh);
				
				if (m_bVerticalFlip)
					m_layerVFlippedQuad->drawElements();
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

		// Vertical Flip
		NodeEditorUI::DrawCheckBoxProperty(
			"drawLayerNodeNodeVerticalflip", 
			"Vertical Flip", 
			m_bVerticalFlip);
	}
}

void DrawLayerNode::onGraphLoaded(bool success)
{
	if (success)
	{
		if (m_materialPin)
		{
			m_materialPin->copyValueFromSourcePin();

			auto materialProperty= std::dynamic_pointer_cast<GraphMaterialProperty>(m_materialPin->getValue());
			if (materialProperty)
			{
				setMaterial(materialProperty->getMaterialResource());
			}

			// Create triangulated quad mesh to draw the layer on
			{
				static QuadVertex x_vertices[] = {
				//   positions                texCoords
					{glm::vec2(-1.0f,  1.0f), glm::vec2(0.0f, 1.0f)},
					{glm::vec2(-1.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
					{glm::vec2( 1.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
					{glm::vec2( 1.0f,  1.0f), glm::vec2(1.0f, 1.0f)},
				};
				static uint16_t x_indices[] = {0, 1, 2, 0, 2, 3};

				m_layerMesh = std::make_shared<GlTriangulatedMesh>("layer_quad_mesh",
																   getVertexDefinition(),
																   (const uint8_t *)x_vertices,
																   4, // 4 verts
																   (const uint8_t *)x_indices,
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
					{glm::vec2( 1.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
					{glm::vec2( 1.0f,  1.0f), glm::vec2(1.0f, 0.0f)},
				};
				static uint16_t x_indices[] = {0, 1, 2, 0, 2, 3};

				m_layerVFlippedQuad = std::make_shared<GlTriangulatedMesh>("layer_vflipped_quad_mesh",
																	 getVertexDefinition(),
																	 (const uint8_t*)x_vertices,
																	 4, // 4 verts
																	 (const uint8_t*)x_indices,
																	 2, // 2 tris
																	 false); // mesh doesn't own quad vert data
				m_layerVFlippedQuad->createBuffers();
			}
		}
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
		}

	}
}

void DrawLayerNode::onLinkDisconnected(NodeLinkPtr link, NodePinPtr pin)
{
	if (pin == m_materialPin)
	{
		setMaterial(GlMaterialPtr());
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

const GlVertexDefinition& DrawLayerNode::getVertexDefinition()
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
	//TODO: Add vertex definition attribute to the pin
	ArrayPinPtr stencilListInPin = node->addPin<ArrayPin>("stencils", eNodePinDirection::INPUT);
	stencilListInPin->setElementClassName(GraphStencilProperty::k_propertyClassName);

	// Create default output pins
	FlowPinPtr flowOutPin = node->addPin<FlowPin>("flowOut", eNodePinDirection::OUTPUT);

	// Listen for Material input pin events
	node->setMaterialPin(materialInPin);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the default pins to a compatible target pin
	autoConnectInputPin(editorState, flowInPin);
	autoConnectOutputPin(editorState, flowOutPin);
	autoConnectInputPin(editorState, materialInPin);
	autoConnectInputPin(editorState, stencilListInPin);

	return node;
}