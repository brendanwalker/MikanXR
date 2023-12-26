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
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "Pins/FloatPin.h"
#include "Pins/FlowPin.h"
#include "Pins/IntPin.h"
#include "Pins/MaterialPin.h"
#include "Pins/ModelPin.h"
#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"
#include "Pins/VariableListPin.h"
#include "Properties/GraphVariableList.h"
#include "Properties/GraphModelProperty.h"
#include "Properties/GraphStencilProperty.h"

#include "imgui.h"
#include "imnodes.h"

#include <glm/gtc/type_ptr.hpp>

#include <typeinfo>
#include <GL/glew.h>

DrawLayerNode::~DrawLayerNode()
{
	setOwnerGraph(NodeGraphPtr());
	//setModelPin(ModelPinPtr());
	setMaterialPin(MaterialPinPtr());
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

void DrawLayerNode::setMaterialPin(MaterialPinPtr inPin)
{
	if (inPin != m_materialPin)
	{
		if (m_materialPin)
		{
			m_materialPin->OnLinkConnected -= MakeDelegate(this, &DrawLayerNode::onMaterialLinkConnected);
			m_materialPin->OnLinkDisconnected -= MakeDelegate(this, &DrawLayerNode::onMaterialLinkDisconnected);
			m_materialPin = nullptr;
		}

		if (inPin)
		{
			inPin->OnLinkConnected += MakeDelegate(this, &DrawLayerNode::onMaterialLinkConnected);
			inPin->OnLinkDisconnected += MakeDelegate(this, &DrawLayerNode::onMaterialLinkDisconnected);
			m_materialPin = inPin;
		}
	}
}

//void DrawLayerNode::setModelPin(ModelPinPtr inPin)
//{
//	if (inPin != m_modelPin)
//	{
//		if (m_modelPin)
//		{
//			m_modelPin->OnLinkConnected -= MakeDelegate(this, &DrawLayerNode::onModelLinkConnected);
//			m_modelPin->OnLinkDisconnected -= MakeDelegate(this, &DrawLayerNode::onModelLinkDisconnected);
//			m_modelPin = nullptr;
//		}
//
//		if (inPin)
//		{
//			inPin->OnLinkConnected += MakeDelegate(this, &DrawLayerNode::onModelLinkConnected);
//			inPin->OnLinkDisconnected += MakeDelegate(this, &DrawLayerNode::onModelLinkDisconnected);
//			m_modelPin = inPin;
//		}
//	}
//}

void DrawLayerNode::setModel(GlRenderModelResourcePtr inModel)
{
	if (inModel != m_model)
	{
		m_model= inModel;

		if (m_model && m_material)
		{
			const auto& triMeshVertexDefinition= m_model->getVertexDefinition();
			const auto& materialVertexDefinition= m_material->getProgram()->getVertexDefinition();

			if (!triMeshVertexDefinition->isCompatibleDefinition(materialVertexDefinition))
			{
				// Clear out the incompatible material
				m_material= GlMaterialPtr();
				rebuildInputPins();
			}
		}
	}
}

void DrawLayerNode::setMaterial(GlMaterialPtr inMaterial)
{
	if (inMaterial != m_material)
	{
		m_material = inMaterial;

		if (m_model && m_material)
		{
			const auto& triMeshVertexDefinition = m_model->getVertexDefinition();
			const auto& materialVertexDefinition = m_material->getProgram()->getVertexDefinition();

			if (!triMeshVertexDefinition->isCompatibleDefinition(materialVertexDefinition))
			{
				// Clear out the incompatible material
				m_material = GlMaterialPtr();
			}
		}

		// Rebuild the pins since the material changed
		rebuildInputPins();
	}
}

bool DrawLayerNode::evaluateNode(NodeEvaluator& evaluator)
{
	bool bSuccess= true;

	if (!m_model)
	{
		evaluator.setLastErrorCode(eNodeEvaluationErrorCode::evaluationError);
		evaluator.setLastErrorMessage("Missing triangulated mesh");
		bSuccess = false;
	}

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
		// Map input pins to the program
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

			// Bind the layer shader program and uniform parameters.
			// This will fail unless all of the shader uniform parameters are bound.
			GlScopedMaterialBinding materialBinding =
				m_material->bindMaterial(
					GlSceneConstPtr(),
					GlCameraConstPtr());

			if (materialBinding)
			{
				for (int i = 0; i < m_model->getTriangulatedMeshCount(); i++)
				{
					auto triMesh= m_model->getTriangulatedMesh(i);

					triMesh->drawElements();
				}
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
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	bool isNodeOpened = ImGui::CollapsingHeader("Draw Layer Node", ImGuiTreeNodeFlags_SpanAvailWidth);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);

	if (isNodeOpened)
	{
		// Material
		ImGui::Text("\t\Material");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

		const std::string material_name = m_material ? m_material->getName() : "<INVALID>";
		ImGui::Text(material_name.c_str());
		ImGui::PopStyleColor();
	}
}

void DrawLayerNode::onGraphLoaded(bool success)
{
	if (success)
	{
		if (m_materialPin)
		{
			m_materialPin->copyValueFromSourcePin();
			setMaterial(m_materialPin->getValue());
		}

		//if (m_modelPin)
		//{
		//	m_modelPin->copyValueFromSourcePin();
		//	setModel(m_modelPin->getValue());
		//}
	}
}

void DrawLayerNode::onMaterialLinkConnected(t_node_link_id id)
{
	if (m_materialPin)
	{
		m_materialPin->copyValueFromSourcePin();
		setMaterial(m_materialPin->getValue());
	}
}

void DrawLayerNode::onMaterialLinkDisconnected(t_node_link_id id)
{
	setMaterial(GlMaterialPtr());
}

//void DrawLayerNode::onModelLinkConnected(t_node_link_id id)
//{
//	if (m_modelPin)
//	{
//		m_modelPin->copyValueFromSourcePin();
//		setModel(m_modelPin->getValue());
//	}
//}

//void DrawLayerNode::onModelLinkDisconnected(t_node_link_id id)
//{
//	setModel(GlRenderModelResourcePtr());
//}

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

// -- ProgramNode Factory -----
NodePtr DrawLayerNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node and default pins
	// The rest of the input pins can't be connected until we have a material assigned
	auto node = std::static_pointer_cast<DrawLayerNode>(NodeFactory::createNode(editorState));

	// Create default input pins
	FlowPinPtr flowInPin = node->addPin<FlowPin>("flowIn", eNodePinDirection::INPUT);
	MaterialPinPtr materialInPin = node->addPin<MaterialPin>("material", eNodePinDirection::INPUT);
	VariableListPinPtr stencilListInPin = node->addPin<VariableListPin>("stencils", eNodePinDirection::INPUT);
	stencilListInPin->setVariableClassName(GraphStencilProperty::k_nodeClassName);

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