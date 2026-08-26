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
static std::string makeImGuiElementName(const std::string& name) { return StringUtils::stringify("##", name); }

bool drawPropertySheetHeader(MkGuiStyleConstPtr style, const std::string headerText)
{
	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	MkGuiScopedStyle headerStyle(style);
	return ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
}

void drawStaticTextProperty(MkGuiStyleConstPtr style, const std::string label, const std::string text)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	MkGuiScopedStyle textStyle(style);
	ImGui::TextUnformatted(text.c_str());
}

bool drawCheckBoxProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
						  bool& inout_value)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::Checkbox(imguiElementName.c_str(), &inout_value);
}

bool drawIntProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label, int& inout_value)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::InputInt(imguiElementName.c_str(), &inout_value);
}

bool drawFloatProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
					   float& inout_value)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::InputFloat(imguiElementName.c_str(), &inout_value);
}

bool drawFloatSliderProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
							 float& inout_value, float srcMin, float srcMax, float displayMin, float displayMax)
{
	const float srcRange= srcMax - srcMin;
	float displayValue=
		(srcRange != 0.0f) ? displayMin + (inout_value - srcMin) / srcRange * (displayMax - displayMin) : displayMin;

	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	if (ImGui::SliderFloat(imguiElementName.c_str(), &displayValue, displayMin, displayMax))
	{
		const float displayRange= displayMax - displayMin;
		inout_value=
			(displayRange != 0.0f) ? srcMin + (displayValue - displayMin) / displayRange * (srcMax - srcMin) : srcMin;
		return true;
	}
	return false;
}

bool drawFloat2Property(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label, float* inout_v)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::InputFloat2(imguiElementName.c_str(), inout_v);
}

bool drawFloat3Property(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label, float* inout_v)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::InputFloat3(imguiElementName.c_str(), inout_v);
}

bool drawFloat4Property(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label, float* inout_v)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::InputFloat4(imguiElementName.c_str(), inout_v);
}

bool drawStringProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label, char* buf,
						size_t bufSize)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::InputText(imguiElementName.c_str(), buf, bufSize, ImGuiInputTextFlags_EnterReturnsTrue);
}

bool drawFilePathProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
						  const std::string& path)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());

	const float browseButtonWidth= 30.f;
	const float pathWidth= style->getValueWidth() - browseButtonWidth - ImGui::GetStyle().ItemSpacing.x;
	ImGui::SetNextItemWidth(pathWidth);
	const std::string imguiPathName= makeImGuiElementName(fieldName + "_path");
	char buf[512];
	strncpy_s(buf, sizeof(buf), path.c_str(), _TRUNCATE);
	ImGui::InputText(imguiPathName.c_str(), buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);

	ImGui::SameLine();
	const std::string imguiBrowseName= makeImGuiElementName(fieldName + "_browse");
	return ImGui::Button(("...##" + imguiBrowseName).c_str(), ImVec2(browseButtonWidth, 0));
}

bool drawSimpleComboBoxProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
								const char* items, int& inout_selectedIdex)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	MkGuiScopedStyle comboStyle(style);
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::Combo(imguiElementName.c_str(), &inout_selectedIdex, items);
}

void drawImageProperty(MkGuiStyleConstPtr style, const std::string label, IMkTextureConstPtr image)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	ImGui::Dummy(ImVec2(1.0f, 0.5f));
	uint32_t glTextureId= image ? image->getGlTextureId() : 0;
	ImGui::Image((ImTextureID)(intptr_t)glTextureId, ImVec2(100, 100));
}

void drawImage(IMkTextureConstPtr image, float width, float height)
{
	uint32_t glTextureId= image ? image->getGlTextureId() : 0;
	ImGui::Image((ImTextureID)(intptr_t)glTextureId, ImVec2(width, height));
}

bool drawImageButton(MkGuiStyleConstPtr style, const std::string& fieldName, const std::string& imageName)
{
	const MkGuiStyleTextureEntry* entry= style->findTexture(imageName);
	if (!entry)
		return false;
	uint32_t glTextureId= entry->texture ? entry->texture->getGlTextureId() : 0;
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::ImageButton(imguiElementName.c_str(), (ImTextureID)(intptr_t)glTextureId, ImVec2(entry->x, entry->y));
}

const char* ComboBoxDataSource::itemGetter(void* data, int idx)
{
	auto* dataSource= (ComboBoxDataSource*)data;

	if (idx >= 0 && idx < dataSource->getEntryCount())
	{
		return dataSource->getEntryDisplayString(idx).c_str();
	}

	return nullptr;
}

bool drawComboBoxProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
						  ComboBoxDataSource* dataSource, int& inout_selectedIdex)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	MkGuiScopedStyle comboStyle(style);
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::Combo(imguiElementName.c_str(), &inout_selectedIdex, &ComboBoxDataSource::itemGetter, dataSource,
						dataSource->getEntryCount());
}

bool drawEnumComboBoxProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
							  const std::vector<std::string>& entries, int& inout_selectedIndex)
{
	ImGui::TextUnformatted(label.c_str());
	ImGui::SameLine(style->getLabelWidth());
	ImGui::SetNextItemWidth(style->getValueWidth());
	MkGuiScopedStyle comboStyle(style);
	const std::string imguiElementName= makeImGuiElementName(fieldName);
	return ImGui::Combo(
		imguiElementName.c_str(), &inout_selectedIndex,
		[](void* data, int n) -> const char*
		{
			auto* v= static_cast<const std::vector<std::string>*>(data);
			if (n >= 0 && n < (int)v->size())
			{
				return (*v)[n].c_str();
			}
			return nullptr;
		},
		(void*)&entries, (int)entries.size());
}

bool drawRadioButtonsProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
							  const std::vector<std::string>& entries, int& inout_selectedIndex)
{
	ImGui::TextUnformatted(label.c_str());
	const int prevIndex= inout_selectedIndex;
	for (int i= 0; i < (int)entries.size(); i++)
	{
		const std::string id= StringUtils::stringify(entries[i], "##", fieldName, "_", i);
		ImGui::RadioButton(id.c_str(), &inout_selectedIndex, i);
	}
	return inout_selectedIndex != prevIndex;
}

ImVec2 mousePosToGridSpace()
{
	ImVec2 canvasOrigin= ImNodes::GetCurrentContext()->CanvasOriginScreenSpace;
	ImVec2 canvasPan= ImNodes::EditorContextGetPanning();
	ImVec2 mousePos= ImGui::GetMousePos();

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

ImVec4 getPinHoveredColor(float alpha) { return ImVec4(0.53f, 0.937f, 0.765f, alpha); }

ImVec4 getBooleanColor(float alpha) { return ImVec4(0.5f, 0.0f, 0.0f, alpha); }

ImVec4 getEnumColor(float alpha) { return ImVec4(0.f, 0.278f, 0.302f, alpha); }

ImVec4 getIntColor(float alpha) { return ImVec4(0.176f, 0.529f, 0.329f, alpha); }

ImVec4 getIntVectorColor(float alpha) { return ImVec4(0.557f, 0.886f, 0.722f, alpha); }

ImVec4 getFloatColor(float alpha) { return ImVec4(0.624f, 0.973f, 0.267f, alpha); }

ImVec4 getFloatVectorColor(float alpha) { return ImVec4(1.f, 0.78f, 0.173f, alpha); }

ImVec4 getMatrixColor(float alpha) { return ImVec4(0.965f, 0.396f, 0.024f, alpha); }

ImVec4 getPropertyColor(float alpha) { return ImVec4(0.f, 0.631f, 0.929f, alpha); }

ImVec4 getTextureColor(float alpha) { return ImVec4(0.6f, 0.263f, 0.969f, alpha); }

ImVec4 getComponentColor(float alpha) { return ImVec4(0.008f, 0.643f, 0.949f, alpha); }

void* receiveDragDropPayload(const std::string& PayloadType)
{
	void* payload= nullptr;

	MkGuiScopedDragDropTarget ddt;
	if (ddt)
	{
		if (const ImGuiPayload* imguiPayload= ImGui::AcceptDragDropPayload(PayloadType.c_str()))
		{
			payload= imguiPayload->Data;
		}
	}

	return payload;
}
}; // namespace MkGui