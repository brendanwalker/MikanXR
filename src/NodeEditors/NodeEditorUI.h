#pragma once

#include <memory>
#include <string>

namespace NodeEditorUI
{
	bool DrawPropertySheetHeader(const std::string headerText);
	void DrawStaticTextProperty(const std::string label, const std::string text);
	void DrawCheckBoxProperty(const std::string fieldName, const std::string label, bool& inout_value);
	bool DrawSimpleComboBoxProperty(
		const std::string fieldName,
		const std::string label,
		const char* items,
		int& inout_selectedIdex);

	class ComboBoxDataSource
	{
	public:
		virtual int getEntryCount()= 0;
		virtual const std::string& getEntryDisplayString(int index)= 0;

		static bool itemGetter(void* data, int idx, const char** out_str);
	};
	bool DrawComboBoxProperty(
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