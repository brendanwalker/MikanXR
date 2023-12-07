#include "ProgramNode.h"
#include "GlFrameBuffer.h"
#include "GlProgram.h"
#include "GlTexture.h"
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

ProgramNode::ProgramNode()
	: Node()
	, m_attachmentsPinsStartId(1)
	, m_drawMode(GL_TRIANGLES)
{
	m_dispatchSize = 0;
}

ProgramNode::ProgramNode(NodeGraphPtr ownerGraph)
	: Node(ownerGraph)
	, m_attachmentsPinsStartId(1)
	, m_drawMode(GL_TRIANGLES)
{
	if (ownerGraph)
	{
		m_frameBufferArrayProperty = ownerGraph->getTypedPropertyByName<FrameBufferArrayProperty>("framebuffers");
		ownerGraph->OnPropertyModifed+= MakeDelegate(this, &ProgramNode::onGraphPropertyChanged);
	}
}

ProgramNode::~ProgramNode()
{
	m_frameBufferArrayProperty= nullptr;
	if (m_ownerGraph)
	{
		m_ownerGraph->OnPropertyModifed -= MakeDelegate(this, &ProgramNode::onGraphPropertyChanged);
	}
}

void ProgramNode::evaluateNode(NodeEvaluator& evaluator)
{
	bool bSuccess= true;

	if (!m_target->bindProgram())
	{
		evaluator.setLastErrorCode(eNodeEvaluationErrorCode::evaluationError);
		evaluator.setLastErrorMessage("Failed to bind gl program");
		bSuccess= false;
	}

	if (bSuccess && !m_framebuffer->bindFrameBuffer())
	{
		evaluator.setLastErrorCode(eNodeEvaluationErrorCode::evaluationError);
		evaluator.setLastErrorMessage("Failed to bind framebuffer");
		bSuccess= false;
	}

	if (bSuccess)
	{
		// Map input pins to the program
		for (auto& pin : m_pinsIn)
		{
			// Copy value from connected pin
			pin->copyValueFromSourcePin();

			if (FloatPinPtr floatPin = std::dynamic_pointer_cast<FloatPin>(pin))
			{
				m_target->setFloatUniform(pin->getName(), floatPin->getValue());
			}
			else if (Float2PinPtr float2Pin = std::dynamic_pointer_cast<Float2Pin>(pin))
			{
				m_target->setVector2Uniform(pin->getName(), glm::make_vec2(float2Pin->getValue().data()));
			}
			else if (Float3PinPtr float3Pin = std::dynamic_pointer_cast<Float3Pin>(pin))
			{
				m_target->setVector3Uniform(pin->getName(), glm::make_vec3(float3Pin->getValue().data()));
			}
			else if (Float4PinPtr float4Pin = std::dynamic_pointer_cast<Float4Pin>(pin))
			{
				m_target->setVector4Uniform(pin->getName(), glm::make_vec4(float3Pin->getValue().data()));
			}
			else if (IntPinPtr floatPin = std::dynamic_pointer_cast<IntPin>(pin))
			{
				m_target->setIntUniform(pin->getName(), floatPin->getValue());
			}
			else if (Int2PinPtr float2Pin = std::dynamic_pointer_cast<Int2Pin>(pin))
			{
				m_target->setInt2Uniform(pin->getName(), glm::make_vec2(float2Pin->getValue().data()));
			}
			else if (Int3PinPtr float3Pin = std::dynamic_pointer_cast<Int3Pin>(pin))
			{
				m_target->setInt3Uniform(pin->getName(), glm::make_vec3(float3Pin->getValue().data()));
			}
			else if (Int4PinPtr float4Pin = std::dynamic_pointer_cast<Int4Pin>(pin))
			{
				m_target->setInt4Uniform(pin->getName(), glm::make_vec4(float3Pin->getValue().data()));
			}
			else if (TexturePinPtr texturePin = std::dynamic_pointer_cast<TexturePin>(pin))
			{
				GlTexturePtr texturePtr = texturePin->getValue();

				if (texturePtr)
				{
					int textureUnit;
					if (m_target->getUniformTextureUnit(pin->getName(), textureUnit))
					{
						texturePtr->bindTexture(textureUnit);
						m_target->setTextureUniform(pin->getName());
					}
				}
			}
		}

		// Dispatch Draw Call
		//glBindVertexArray(VAO);
		glDrawArrays(m_drawMode, 0, m_dispatchSize);
		//glBindVertexArray(0);
	}

	// Clean up any active bindings
	// TODO Texture Bindings
	m_framebuffer->unbindFrameBuffer();
	m_target->unbindProgram();
}

FlowPinPtr ProgramNode::getOutputFlowPin() const
{
	return getFirstPinOfType<FlowPin>(eNodePinDirection::OUTPUT);
}

std::string ProgramNode::editorGetTitle() const
{ 
	return m_target ? m_target->getProgramCode().getProgramName() : "Program";
}

void ProgramNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// title bar
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	bool isNodeOpened = ImGui::CollapsingHeader("Program Node", ImGuiTreeNodeFlags_SpanAvailWidth);
	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);

	if (isNodeOpened)
	{
		// Framebuffer
		ImGui::Text("\t\tFramebuffer");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));

		const std::string& frameBufferName = m_framebuffer ? m_framebuffer->getName() : "<INVALID>";
		if (ImGui::BeginCombo("##progNodeFramebuffer", frameBufferName.c_str()))
		{
			if (m_frameBufferArrayProperty)
			{
				int index = 0;
				for (auto& framebuffer : m_frameBufferArrayProperty->getArray())
				{
					const bool is_selected = (m_framebuffer == framebuffer);
					if (ImGui::Selectable(framebuffer->getName().c_str(), is_selected))
					{
						setFramebuffer(framebuffer);
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

		// Draw mode
		ImGui::Text("\t\tDraw Mode");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		int iVal = EditorNodeUtil::GLDrawModeToIndex(m_drawMode);
		const char* items =
			"Points\0"
			"Line Strip\0"
			"Line Loop\0"
			"Lines\0"
			"Line Strip Adjacency\0"
			"Lines Adjacency\0"
			"Triangle Strip\0"
			"Triangle Fan\0"
			"Triangles\0"
			"Triangle Strip Adjacency\0"
			"Triangles Adjacency\0";
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
		if (ImGui::Combo("##progNodeDrawMode", &iVal, items))
		{
			m_drawMode = EditorNodeUtil::IndexToGLDrawMode(iVal);
		}
		ImGui::PopStyleColor();

		// Size
		ImGui::Text("\t\tSize");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		iVal = m_dispatchSize;
		if (ImGui::DragInt("##progNodeDrawSize", &iVal, 1.0f))
		{
			if (iVal < 0) iVal = 0;
			m_dispatchSize = iVal;
		}
	}
}

void ProgramNode::setFramebuffer(GlFrameBufferPtr inFrameBuffer)
{
	if (inFrameBuffer == m_framebuffer)
		return;

	for (auto it = m_pinsOut.begin(); it != m_pinsOut.end(); it++)
	{
		TexturePinPtr outPin= std::dynamic_pointer_cast<TexturePin>(*it);

		if (outPin)
		{
			getOwnerGraph()->deletePinById(outPin->getId());
		}
	}

	if (inFrameBuffer)
	{
		for (int i = 0; i < inFrameBuffer->getNumAttachments(); i++)
		{
			addPin<TexturePin>("Attachment " + std::to_string(i), eNodePinDirection::OUTPUT);
		}
	}

	m_framebuffer = inFrameBuffer;
}

void ProgramNode::onGraphPropertyChanged(t_graph_property_id id)
{
	if (m_frameBufferArrayProperty && m_frameBufferArrayProperty->getId() == id)
	{
		auto frameBufferArray= m_frameBufferArrayProperty->getArray();
		if (std::find(frameBufferArray.begin(), frameBufferArray.end(), m_framebuffer) == frameBufferArray.end())
		{
			setFramebuffer(GlFrameBufferPtr());
		}
	}
}