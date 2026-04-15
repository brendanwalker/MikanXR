#pragma once

#include <memory>
#include <string>

class MkGuiStyleManager;

struct ImVec2;
struct ImVec4;

class MkGuiStyle;
using MkGuiStyleConstPtr = std::shared_ptr<const MkGuiStyle>;

class IMkTexture;
using IMkTextureConstPtr = std::shared_ptr<const IMkTexture>;

namespace MkGui
{
	bool drawPropertySheetHeader(
		MkGuiStyleConstPtr style,
		const std::string headerText);
	void drawStaticTextProperty(
		MkGuiStyleConstPtr style,
		const std::string label, 
		const std::string text);
	bool drawCheckBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		bool& inout_value);
	bool drawIntProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		int& inout_value);
	bool drawFloatProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float& inout_value);
	bool drawFloatSliderProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float& inout_value,
		float srcMin, float srcMax,
		float displayMin, float displayMax);
	bool drawFloat2Property(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float* inout_v);
	bool drawFloat3Property(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float* inout_v);
	bool drawFloat4Property(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		float* inout_v);
	bool drawStringProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		char* buf,
		size_t bufSize);
	bool drawSimpleComboBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		const char* items,
		int& inout_selectedIdex);
	void drawImageProperty(
		MkGuiStyleConstPtr style,
		const std::string label,
		IMkTextureConstPtr image);
	void drawImage(
		IMkTextureConstPtr image,
		float width,
		float height);

	class ComboBoxDataSource
	{
	public:
		virtual int getEntryCount() const = 0;
		virtual const std::string& getEntryDisplayString(int index) const = 0;

		static bool itemGetter(void* data, int idx, const char** out_str);
	};
	bool drawComboBoxProperty(
		MkGuiStyleConstPtr style,
		const std::string fieldName,
		const std::string label,
		ComboBoxDataSource* dataSource,
		int& inout_selectedIdex);

	void* receiveDragDropPayload(const std::string& PayloadType);
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