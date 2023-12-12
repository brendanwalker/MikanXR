#include "DrawTriMeshNode.h"
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
#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"
#include "Properties/GraphArrayProperty.h"

#include "imgui.h"
#include "imnodes.h"

#include <glm/gtc/type_ptr.hpp>

#include <typeinfo>
#include <GL/glew.h>

DrawTriMeshNode::DrawTriMeshNode()
	: Node()
{
}

DrawTriMeshNode::DrawTriMeshNode(NodeGraphPtr ownerGraph)
	: Node(ownerGraph)
{
	if (ownerGraph)
	{
		m_modelArrayProperty = ownerGraph->getTypedPropertyByName<ModelResourceArrayProperty>("models");
		ownerGraph->OnPropertyModifed+= MakeDelegate(this, &DrawTriMeshNode::onGraphPropertyChanged);
	}
}

DrawTriMeshNode::~DrawTriMeshNode()
{
	m_modelArrayProperty= nullptr;
	if (m_ownerGraph)
	{
		m_ownerGraph->OnPropertyModifed -= MakeDelegate(this, &DrawTriMeshNode::onGraphPropertyChanged);
	}
}

void DrawTriMeshNode::setModel(GlRenderModelResourcePtr inModel)
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

void DrawTriMeshNode::setMaterial(GlMaterialPtr inMaterial)
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

bool DrawTriMeshNode::evaluateNode(NodeEvaluator& evaluator)
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

FlowPinPtr DrawTriMeshNode::getOutputFlowPin() const
{
	return getFirstPinOfType<FlowPin>(eNodePinDirection::OUTPUT);
}

std::string DrawTriMeshNode::editorGetTitle() const
{ 
	return m_material ? m_material->getName() : "Draw Tri Mesh";
}

void DrawTriMeshNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// title bar
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	bool isNodeOpened = ImGui::CollapsingHeader("Draw Tri Mesh Node", ImGuiTreeNodeFlags_SpanAvailWidth);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);

	if (isNodeOpened)
	{
		// Model
		ImGui::Text("\t\tModel");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

		const std::string& modelName = m_model ? m_model->getName() : "<INVALID>";
		if (ImGui::BeginCombo("##modelSelection", modelName.c_str()))
		{
			if (m_modelArrayProperty)
			{
				int index = 0;
				for (auto& model : m_modelArrayProperty->getArray())
				{
					const bool is_selected = (m_model == model);
					if (ImGui::Selectable(model->getName().c_str(), is_selected))
					{
						setModel(model);
					}

					if (is_selected)
					{
						ImGui::SetItemDefaultFocus();
					}

					index++;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopStyleColor();
	}
}

void DrawTriMeshNode::rebuildInputPins()
{
	// Delete all pins except flow pins
	for (auto it = m_pinsIn.begin(); it != m_pinsIn.end(); it++)
	{
		NodePinPtr pin = *it;
		
		if (!std::dynamic_pointer_cast<FlowPin>(pin))
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
			eUniformDataType uniformDataType = GlProgram::getUniformSemanticDataType(uniformSemantic);

			switch (uniformDataType)
			{
				case eUniformDataType::datatype_float:
					{
						addPin<FloatPin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				case eUniformDataType::datatype_float2:
					{
						addPin<Float2Pin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				case eUniformDataType::datatype_float3:
					{
						addPin<Float3Pin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				case eUniformDataType::datatype_float4:
					{
						addPin<Float4Pin>(uniformName, eNodePinDirection::INPUT);
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
						addPin<TexturePin>(uniformName, eNodePinDirection::INPUT);
					}
					break;
				default:
					assert(false);
			}
		}
	}
}

void DrawTriMeshNode::onGraphPropertyChanged(t_graph_property_id id)
{
	if (m_modelArrayProperty && m_modelArrayProperty->getId() == id)
	{
		auto modelResourceArray= m_modelArrayProperty->getArray();
		if (std::find(modelResourceArray.begin(), modelResourceArray.end(), m_model) == modelResourceArray.end())
		{
			setModel(GlRenderModelResourcePtr());
		}
	}
}

// -- ProgramNode Factory -----
NodePtr DrawTriMeshNodeFactory::createNode(const NodeEditorState* editorState) const
{
	// Create the node and default pins
	// The rest of the input pins can't be connected until we have a material assigned
	ProgramNodePtr node = std::make_shared<DrawTriMeshNode>();
	FlowPinPtr flowInPin = node->addPin<FlowPin>("flowIn", eNodePinDirection::INPUT);
	FlowPinPtr flowOutPin = node->addPin<FlowPin>("flowOut", eNodePinDirection::OUTPUT);

	// If spawned in an editor context from a dangling pin link
	// auto-connect the flow pins to a compatible target pin
	autoConnectInputPin(editorState, flowInPin);
	autoConnectOutputPin(editorState, flowOutPin);

	return node;
}