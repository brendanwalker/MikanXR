#include "ProgramNode.h"
#include "GlFrameBuffer.h"
#include "GlProgram.h"
#include "EditorNodeUtil.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodePin.h"
#include "Pins/TexturePin.h"
#include "Properties/GraphArrayProperty.h"

#include "imgui.h"
#include "imnodes.h"

#include <typeinfo>
#include <GL/glew.h>

ProgramNode::ProgramNode()
	: Node()
	, m_attachmentsPinsStartId(1)
	, m_dispatchType(eProgramDispatchType::ARRAY)
	, m_drawMode(GL_POINTS)
{
	m_dispatchSize[3] = {};
}

ProgramNode::ProgramNode(NodeGraphPtr ownerGraph)
	: Node(ownerGraph)
	, m_attachmentsPinsStartId(1)
	, m_dispatchType(eProgramDispatchType::ARRAY)
	, m_drawMode(GL_POINTS)
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
		// Dispatch type
		ImGui::Text("\t\tDispatch Type");
		ImGui::SameLine(160);
		ImGui::SetNextItemWidth(150);
		int iVal = (int)m_dispatchType;
		const char* items = "Array\0Compute\0";
		ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
		if (ImGui::Combo("##progNodeDispatchType", &iVal, items))
		{
			m_drawMode = GL_POINTS;
			if (iVal == 0)
			{
				m_dispatchType = eProgramDispatchType::ARRAY;
				m_dispatchSize[0] = 0;
				m_dispatchSize[1] = 0;
				m_dispatchSize[2] = 0;
			}
			else if (iVal == 1)
			{
				m_dispatchType = eProgramDispatchType::COMPUTE;
				m_dispatchSize[0] = 1;
				m_dispatchSize[1] = 1;
				m_dispatchSize[2] = 1;
				setFramebuffer(GlFrameBufferPtr());
			}
		}
		ImGui::PopStyleColor();

		if (m_dispatchType == eProgramDispatchType::ARRAY)
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
			iVal = EditorNodeUtil::GLDrawModeToIndex(m_drawMode);
			items =
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
			iVal = m_dispatchSize[0];
			if (ImGui::DragInt("##progNodeDrawSize", &iVal, 1.0f))
			{
				if (iVal < 0) iVal = 0;
				m_dispatchSize[0] = iVal;
			}
		}
		else if (m_dispatchType == eProgramDispatchType::COMPUTE)
		{
			int size[3]{};
			size[0] = m_dispatchSize[0];
			size[1] = m_dispatchSize[1];
			size[2] = m_dispatchSize[2];
			// Dispatch size
			ImGui::Text("\t\tWork Group Size X");
			ImGui::SameLine(160);
			ImGui::SetNextItemWidth(150);
			if (ImGui::DragInt("##progNodeWorkGroupSizeX", &size[0], 1.0f))
			{
				if (size[0] < 0) size[0] = 0;
				m_dispatchSize[0] = size[0];
			}
			ImGui::Text("\t\tWork Group Size Y");
			ImGui::SameLine(160);
			ImGui::SetNextItemWidth(150);
			if (ImGui::DragInt("##progNodeWorkGroupSizeY", &size[1], 1.0f))
			{
				if (size[1] < 0) size[1] = 0;
				m_dispatchSize[1] = size[1];
			}
			ImGui::Text("\t\tWork Group Size Z");
			ImGui::SameLine(160);
			ImGui::SetNextItemWidth(150);
			if (ImGui::DragInt("##progNodeWorkGroupSizeZ", &size[2], 1.0f))
			{
				if (size[2] < 0) size[2] = 0;
				m_dispatchSize[2] = size[2];
			}
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