#include "MkGuiDrawUtils.h"
#include "MkGuiScopedStyle.h"
#include "MkGuiStyleManager.h"
#include "MkGuiScopedDragDropTarget.h"
#include "StringUtils.h"
#include "IMkTexture.h"

#include "imnodes.h"
#include "imnodes_internal.h"

#include "IconsForkAwesome.h"

namespace MkGui
{
	static std::string makeImGuiElementName(const std::string& name)
	{
		return StringUtils::stringify("##", name);
	}

	bool drawPropertySheetHeader(
		MkGuiStyleConstPtr style,
		const std::string headerText)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		MkGuiScopedStyle headerStyle(style);
		return ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
	}

	void drawStaticTextProperty(
		MkGuiStyleConstPtr style,
		const std::string label, 
		const std::string text)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(style->labelWidth);
		ImGui::SetNextItemWidth(style->valueWidth);
		MkGuiScopedStyle textStyle(style);
		ImGui::Text(text.c_str());
	}

	void drawCheckBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName, 
		const std::string label, 
		bool& inout_value)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(style->labelWidth);
		ImGui::SetNextItemWidth(style->valueWidth);
		const std::string imguiElementName= makeImGuiElementName(fieldName);
		ImGui::Checkbox(imguiElementName.c_str(), &inout_value);
	}

	bool drawSimpleComboBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		const char* items,
		int& inout_selectedIdex)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(style->labelWidth);
		ImGui::SetNextItemWidth(style->valueWidth);
		MkGuiScopedStyle comboStyle(style);
		const std::string imguiElementName = makeImGuiElementName(fieldName);
		return ImGui::Combo(imguiElementName.c_str(), &inout_selectedIdex, items);
	}

	void drawImageProperty(
		MkGuiStyleConstPtr style,
		const std::string label, 
		IMkTextureConstPtr image)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(style->labelWidth);
		ImGui::SetNextItemWidth(style->valueWidth);
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

	bool drawComboBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		ComboBoxDataSource* dataSource,
		int& inout_selectedIdex)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(style->labelWidth);
		ImGui::SetNextItemWidth(style->valueWidth);
		MkGuiScopedStyle comboStyle(style);
		const std::string imguiElementName = makeImGuiElementName(fieldName);
		return ImGui::Combo(imguiElementName.c_str(),
			&inout_selectedIdex,
			&ComboBoxDataSource::itemGetter,
			dataSource,
			dataSource->getEntryCount());
	}

	void* receiveDragDropPayload(const std::string& PayloadType)
	{
		void* payload= nullptr;

		MkGuiScopedDragDropTarget ddt;
		if (ddt)
		{
			if (const ImGuiPayload* imguiPayload = ImGui::AcceptDragDropPayload(PayloadType.c_str()))
			{
				payload = imguiPayload->Data;
			}
		}

		return payload;
	}
};