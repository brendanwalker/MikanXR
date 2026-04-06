#include "NodeEditorUI.h"
#include "MkGuiScopedStyle.h"
#include "MkGuiStyleManager.h"
#include "MkGuiScopedDragDropTarget.h"
#include "StringUtils.h"
#include "IMkTexture.h"

#include "imnodes.h"
#include "imnodes_internal.h"

#include "IconsForkAwesome.h"

namespace NodeEditorUI
{
	const int k_labelWidth= 100;
	const int k_valueWidth= 150;
	const ImVec4 k_valueBGColor(0.13f, 0.13f, 0.13f, 1.0f);

	ImVec2 MousePosToGridSpace()
	{
		ImVec2 canvasOrigin = ImNodes::GetCurrentContext()->CanvasOriginScreenSpace;
		ImVec2 canvasPan = ImNodes::EditorContextGetPanning();
		ImVec2 mousePos = ImGui::GetMousePos();

		return mousePos - canvasOrigin - canvasPan;
	}

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

	bool DrawPropertySheetHeader(const std::string headerText, MkGuiStyleManager* styleManager)
	{
		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		MkGuiScopedStyle headerStyle(styleManager->getStyle("node_editor_panel_header"));
		return ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
	}

	void DrawStaticTextProperty(const std::string label, const std::string text, MkGuiStyleManager* styleManager)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(k_labelWidth);
		ImGui::SetNextItemWidth(k_valueWidth);
		MkGuiScopedStyle textStyle(styleManager->getStyle("node_editor_property_value"));
		ImGui::Text(text.c_str());
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
		int& inout_selectedIdex,
		MkGuiStyleManager* styleManager)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(k_labelWidth);
		ImGui::SetNextItemWidth(k_valueWidth);
		MkGuiScopedStyle comboStyle(styleManager->getStyle("node_editor_property_value"));
		const std::string imguiElementName = makeImGuiElementName(fieldName);
		return ImGui::Combo(imguiElementName.c_str(), &inout_selectedIdex, items);
	}

	void DrawImageProperty(const std::string label, IMkTexturePtr image)
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
		int& inout_selectedIdex,
		MkGuiStyleManager* styleManager)
	{
		ImGui::Text(label.c_str());
		ImGui::SameLine(k_labelWidth);
		ImGui::SetNextItemWidth(k_valueWidth);
		MkGuiScopedStyle comboStyle(styleManager->getStyle("node_editor_property_value"));
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