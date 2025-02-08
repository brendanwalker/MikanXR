#pragma once

#include "MikanRendererFwd.h"

#include <memory>
#include <string>

struct ImVec2;
struct ImVec4;

namespace NodeEditorUI
{
	ImVec2 MousePosToGridSpace();

	const std::string& getVariableIcon();
	const std::string& getArrayIcon();

	const ImVec4 getPinHoveredColor(float alpha = 1.f);
	const ImVec4 getBooleanColor(float alpha = 1.f);
	const ImVec4 getEnumColor(float alpha = 1.f);
	const ImVec4 getIntColor(float alpha = 1.f);
	const ImVec4 getIntVectorColor(float alpha = 1.f);
	const ImVec4 getFloatColor(float alpha = 1.f);
	const ImVec4 getFloatVectorColor(float alpha = 1.f);
	const ImVec4 getMatrixColor(float alpha = 1.f);
	const ImVec4 getPropertyColor(float alpha = 1.f);
	const ImVec4 getTextureColor(float alpha = 1.f);
	const ImVec4 getComponentColor(float alpha = 1.f);

	bool DrawPropertySheetHeader(const std::string headerText);
	void DrawStaticTextProperty(const std::string label, const std::string text);
	void DrawCheckBoxProperty(const std::string fieldName, const std::string label, bool& inout_value);
	bool DrawSimpleComboBoxProperty(
		const std::string fieldName,
		const std::string label,
		const char* items,
		int& inout_selectedIdex);
	void DrawImageProperty(const std::string label, IMkTexturePtr image);

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