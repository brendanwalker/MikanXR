#include "NodeEditorUI.h"
#include "StringUtils.h"
#include "GlTexture.h"


#include "imgui.h"

namespace NodeEditorUI
{
	const int k_labelWidth= 100;
	const int k_valueWidth= 150;
	const ImVec4 k_valueBGColor(0.13f, 0.13f, 0.13f, 1.0f);

	static std::string makeImGuiElementName(const std::string& name)
	{
		return StringUtils::stringify("##", name);
	}

	bool DrawPropertySheetHeader(const std::string headerText)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
		bool isNodeOpened = ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor(3);

		return isNodeOpened;
	}

	void DrawStaticTextProperty(const std::string label, const std::string text)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(k_labelWidth);
		ImGui::SetNextItemWidth(k_valueWidth);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, k_valueBGColor);
		ImGui::Text(text.c_str());
		ImGui::PopStyleColor();
	}

	void DrawCheckBoxProperty(const std::string fieldName, const std::string label, bool& inout_value)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(k_labelWidth);
		ImGui::SetNextItemWidth(k_valueWidth);
		const std::string imguiElementName= makeImGuiElementName(fieldName);
		ImGui::Checkbox(imguiElementName.c_str(), &inout_value);
	}

	bool DrawSimpleComboBoxProperty(
		const std::string fieldName,
		const std::string label,
		const char* items,
		int& inout_selectedIdex)
	{

		ImGui::Text(label.c_str());
		ImGui::SameLine(k_labelWidth);
		ImGui::SetNextItemWidth(k_valueWidth);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, k_valueBGColor);
		const std::string imguiElementName = makeImGuiElementName(fieldName);
		const bool bChanged = ImGui::Combo(imguiElementName.c_str(), &inout_selectedIdex, items);
		ImGui::PopStyleColor();

		return bChanged;
	}

	void DrawImageProperty(const std::string label, GlTexturePtr image)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(k_labelWidth);
		ImGui::SetNextItemWidth(k_valueWidth);
		ImGui::Dummy(ImVec2(1.0f, 0.5f));
		uint32_t glTextureId = image ? image->getGlTextureId() : 0;
		ImGui::Image((void*)(intptr_t)glTextureId, ImVec2(100, 100));
	}

	bool ComboBoxDataSource::itemGetter(void* data, int idx, const char** out_str)
	{
		auto* dataSource = (ComboBoxDataSource*)data;

		if (idx >= 0 && idx < dataSource->getEntryCount())
		{
			*out_str = dataSource->getEntryDisplayString(idx).c_str();
			return true;
		}

		return false;
	}

	bool DrawComboBoxProperty(
		const std::string fieldName,
		const std::string label,
		ComboBoxDataSource* dataSource,
		int& inout_selectedIdex)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(k_labelWidth);
		ImGui::SetNextItemWidth(k_valueWidth);
		ImGui::PushStyleColor(ImGuiCol_PopupBg, k_valueBGColor);
		const std::string imguiElementName = makeImGuiElementName(fieldName);
		const bool bChanged = 
			ImGui::Combo(imguiElementName.c_str(),
				&inout_selectedIdex,
				&ComboBoxDataSource::itemGetter, 
				dataSource, 
				dataSource->getEntryCount());
		ImGui::PopStyleColor();

		return bChanged;
	}

	void* receiveDragDropPayload(const std::string& PayloadType)
	{
		void* payload= nullptr;

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* imguiPayload = ImGui::AcceptDragDropPayload(PayloadType.c_str()))
			{
				payload = imguiPayload->Data;
			}

			ImGui::EndDragDropTarget();
		}

		return payload;
	}
};