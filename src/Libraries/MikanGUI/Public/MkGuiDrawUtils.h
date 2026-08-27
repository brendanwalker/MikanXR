#pragma once

#include "MkGuiExport.h"
#include "IMkGuiStyle.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

class IMkTexture;
using IMkTextureConstPtr= std::shared_ptr<const IMkTexture>;

namespace MkGui
{
MIKAN_GUI_FUNC(bool) drawPropertySheetHeader(MkGuiStyleConstPtr style, const std::string headerText);
MIKAN_GUI_FUNC(void) drawStaticTextProperty(MkGuiStyleConstPtr style, const std::string label, const std::string text);
MIKAN_GUI_FUNC(bool) drawCheckBoxProperty(MkGuiStyleConstPtr style, const std::string fieldName,
										  const std::string label, bool& inout_value);
MIKAN_GUI_FUNC(bool) drawIntProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
									 int& inout_value);
MIKAN_GUI_FUNC(bool) drawFloatProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
									   float& inout_value);
MIKAN_GUI_FUNC(bool) drawFloatSliderProperty(MkGuiStyleConstPtr style, const std::string fieldName,
											 const std::string label, float& inout_value, float srcMin, float srcMax,
											 float displayMin, float displayMax);
MIKAN_GUI_FUNC(bool) drawFloat2Property(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
										float* inout_v);
MIKAN_GUI_FUNC(bool) drawFloat3Property(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
										float* inout_v);
MIKAN_GUI_FUNC(bool) drawFloat4Property(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
										float* inout_v);
MIKAN_GUI_FUNC(bool) drawStringProperty(MkGuiStyleConstPtr style, const std::string fieldName, const std::string label,
										char* buf, size_t bufSize);
MIKAN_GUI_FUNC(bool) drawFilePathProperty(MkGuiStyleConstPtr style, const std::string fieldName,
										  const std::string label, const std::string& path);
MIKAN_GUI_FUNC(bool) drawSimpleComboBoxProperty(MkGuiStyleConstPtr style, const std::string fieldName,
												const std::string label, const char* items, int& inout_selectedIdex);
MIKAN_GUI_FUNC(void) drawImageProperty(MkGuiStyleConstPtr style, const std::string label, IMkTextureConstPtr image);
MIKAN_GUI_FUNC(void) drawImage(IMkTextureConstPtr image, float width, float height);
MIKAN_GUI_FUNC(bool) drawImageButton(MkGuiStyleConstPtr style, const std::string& fieldName,
									 const std::string& imageName);

class MIKAN_GUI_CLASS ComboBoxDataSource
{
public:
	virtual int getEntryCount() const= 0;
	virtual const std::string& getEntryDisplayString(int index) const= 0;

	static const char* itemGetter(void* data, int idx);
};
MIKAN_GUI_FUNC(bool) drawComboBoxProperty(MkGuiStyleConstPtr style, const std::string fieldName,
										  const std::string label, ComboBoxDataSource* dataSource,
										  int& inout_selectedIdex);
MIKAN_GUI_FUNC(bool) drawEnumComboBoxProperty(MkGuiStyleConstPtr style, const std::string fieldName,
											  const std::string label, const std::vector<std::string>& entries,
											  int& inout_selectedIndex);
MIKAN_GUI_FUNC(bool) drawRadioButtonsProperty(MkGuiStyleConstPtr style, const std::string fieldName,
											  const std::string label, const std::vector<std::string>& entries,
											  int& inout_selectedIndex);

MIKAN_GUI_FUNC(ImVec2) mousePosToGridSpace();

MIKAN_GUI_FUNC(const std::string&) getVariableIcon();
MIKAN_GUI_FUNC(const std::string&) getArrayIcon();

MIKAN_GUI_FUNC(ImVec4) getPinHoveredColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getBooleanColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getEnumColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getIntColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getIntVectorColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getFloatColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getFloatVectorColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getMatrixColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getPropertyColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getTextureColor(float alpha= 1.f);
MIKAN_GUI_FUNC(ImVec4) getComponentColor(float alpha= 1.f);

// Receives a drag-drop payload of the given type, invoking the callback with the
// payload bytes. The callback must copy what it needs rather than retain the
// pointer: ImGui frees its payload buffer as the drop target closes, and the
// delivery frame is the only frame the payload is handed out at all.
MIKAN_GUI_FUNC(bool)
receiveDragDropPayload(const std::string& payloadType, const std::function<void(const void*)>& onPayloadReceived);

template <class t_payload_type>
std::shared_ptr<t_payload_type> receiveTypedDragDropPayload(const std::string& PayloadType)
{
	std::shared_ptr<t_payload_type> result;

	receiveDragDropPayload(PayloadType, [&result](const void* payloadData)
						   { result= *reinterpret_cast<const std::shared_ptr<t_payload_type>*>(payloadData); });

	return result;
}
}; // namespace MkGui
