#pragma once

#include "MkGuiExport.h"
#include "IMkGuiStyle.h"

#include <memory>
#include <string>
#include <vector>

class IMkTexture;
using IMkTextureConstPtr = std::shared_ptr<const IMkTexture>;

namespace MkGui
{
	MIKAN_GUI_FUNC(bool) drawPropertySheetHeader(
		MkGuiStyleConstPtr style,
		const std::string headerText);
	MIKAN_GUI_FUNC(void) drawStaticTextProperty(
		MkGuiStyleConstPtr style,
		const std::string label,
		const std::string text);
	MIKAN_GUI_FUNC(bool) drawCheckBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		bool& inout_value);
	MIKAN_GUI_FUNC(bool) drawIntProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		int& inout_value);
	MIKAN_GUI_FUNC(bool) drawFloatProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float& inout_value);
	MIKAN_GUI_FUNC(bool) drawFloatSliderProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float& inout_value,
		float srcMin, float srcMax,
		float displayMin, float displayMax);
	MIKAN_GUI_FUNC(bool) drawFloat2Property(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float* inout_v);
	MIKAN_GUI_FUNC(bool) drawFloat3Property(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float* inout_v);
	MIKAN_GUI_FUNC(bool) drawFloat4Property(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float* inout_v);
	MIKAN_GUI_FUNC(bool) drawStringProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		char* buf,
		size_t bufSize);
	MIKAN_GUI_FUNC(bool) drawFilePathProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		const std::string& path);
	MIKAN_GUI_FUNC(bool) drawSimpleComboBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		const char* items,
		int& inout_selectedIdex);
	MIKAN_GUI_FUNC(void) drawImageProperty(
		MkGuiStyleConstPtr style,
		const std::string label,
		IMkTextureConstPtr image);
	MIKAN_GUI_FUNC(void) drawImage(
		IMkTextureConstPtr image,
		float width,
		float height);
	MIKAN_GUI_FUNC(bool) drawImageButton(
		MkGuiStyleConstPtr style,
		const std::string& fieldName,
		const std::string& imageName);

	class MIKAN_GUI_CLASS ComboBoxDataSource
	{
	public:
		virtual int getEntryCount() const = 0;
		virtual const std::string& getEntryDisplayString(int index) const = 0;

		static bool itemGetter(void* data, int idx, const char** out_str);
	};
	MIKAN_GUI_FUNC(bool) drawComboBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		ComboBoxDataSource* dataSource,
		int& inout_selectedIdex);
	MIKAN_GUI_FUNC(bool) drawEnumComboBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		const std::vector<std::string>& entries,
		int& inout_selectedIndex);
	MIKAN_GUI_FUNC(bool) drawRadioButtonsProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		const std::vector<std::string>& entries,
		int& inout_selectedIndex);

	MIKAN_GUI_FUNC(void*) receiveDragDropPayload(const std::string& PayloadType);
	template <class t_payload_type>
	std::shared_ptr<t_payload_type> receiveTypedDragDropPayload(const std::string& PayloadType)
	{
		void* payload= receiveDragDropPayload(PayloadType);

		if (payload)
		{
			return *reinterpret_cast<std::shared_ptr<t_payload_type>*>(payload);
		}
		else
		{
			return std::shared_ptr<t_payload_type>();
		}
	}
};
