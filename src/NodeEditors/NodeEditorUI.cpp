#include "NodeEditorUI.h"
#include "StringUtils.h"
#include "GlTexture.h"

#include "imgui.h"

#include "IconsForkAwesome.h"

namespace NodeEditorUI
{
	const int k_labelWidth= 100;
	const int k_valueWidth= 150;
	const ImVec4 k_valueBGColor(0.13f, 0.13f, 0.13f, 1.0f);

	const std::string& getVariableIcon()
	{
		static std::string icon= ICON_FK_SQUARE;
		return icon;		
	}

	const std::string& getArrayIcon()
	{
		static std::string icon= ICON_FK_TH;
		return icon;
	}

	const ImVec4 getPinHoveredColor(float alpha)
	{
		return ImVec4(0.53f, 0.937f, 0.765f, alpha);
	}

	const ImVec4 getBooleanColor(float alpha)
	{
		return ImVec4(0.5f, 0.0f, 0.0f, alpha);
	}

	const ImVec4 getEnumColor(float alpha)
	{
		return ImVec4(0.f, 0.278f, 0.302f, alpha);
	}

	const ImVec4 getIntColor(float alpha)
	{
		return ImVec4(0.176f, 0.529f, 0.329f, alpha);
	}

	const ImVec4 getIntVectorColor(float alpha)
	{
		return ImVec4(0.557f, 0.886f, 0.722f, alpha);
	}

	const ImVec4 getFloatColor(float alpha)
	{
		return ImVec4(0.624f, 0.973f, 0.267f, alpha);
	}

	const ImVec4 getFloatVectorColor(float alpha)
	{
		return ImVec4(1.f, 0.78f, 0.173f, alpha);
	}

	const ImVec4 getMatrixColor(float alpha)
	{
		return ImVec4(0.965f, 0.396f, 0.024f, alpha);
	}

	const ImVec4 getPropertyColor(float alpha)
	{
		return ImVec4(0.f, 0.631f, 0.929f, alpha);
	}

	const ImVec4 getTextureColor(float alpha)
	{
		return ImVec4(0.6f, 0.263f, 0.969f, alpha);
	}

	const ImVec4 getComponentColor(float alpha)
	{
		return ImVec4(0.008f, 0.643f, 0.949f, alpha);
	}

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