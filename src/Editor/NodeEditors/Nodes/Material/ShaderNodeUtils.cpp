#include "ShaderNodeUtils.h"
#include "MkGuiDrawUtils.h"
#include "StringUtils.h"

#include "imgui.h"

#include <algorithm>
#include <cctype>

namespace ShaderNodeUtils
{
std::string formatValueText(eShaderValueType type, const ShaderValueDefault& value)
{
	const int componentCount= ShaderValueTypeUtils::getComponentCount(type);
	if (componentCount <= 0)
		return std::string();

	char componentText[32];
	std::string text= (componentCount > 1) ? "(" : "";
	for (int index= 0; index < componentCount; ++index)
	{
		StringUtils::formatString(componentText, sizeof(componentText), "%.2f", value[index]);
		if (index > 0)
			text+= ", ";
		text+= componentText;
	}
	if (componentCount > 1)
		text+= ")";

	return text;
}

bool isValidIdentifier(const std::string& name)
{
	if (name.empty())
		return false;

	if (std::isdigit((unsigned char)name[0]))
		return false;

	for (char c : name)
	{
		if (!std::isalnum((unsigned char)c) && c != '_')
			return false;
	}

	return true;
}

bool drawValueProperty(MkGuiStyleConstPtr style, const std::string& fieldName, const std::string& label,
					   eShaderValueType type, bool bIsColor, ShaderValueDefault& inoutValue)
{
	if (bIsColor)
	{
		ImGui::TextUnformatted(label.c_str());
		ImGui::SameLine((float)style->getLabelWidth());
		ImGui::SetNextItemWidth(std::max((float)style->getValueWidth(), ImGui::GetContentRegionAvail().x));
		const std::string elementName= StringUtils::stringify("##", fieldName);
		return ImGui::ColorEdit4(elementName.c_str(), inoutValue.data());
	}

	switch (ShaderValueTypeUtils::getComponentCount(type))
	{
	case 1:
		return MkGui::drawFloatProperty(style, fieldName, label, inoutValue[0]);
	case 2:
		return MkGui::drawFloat2Property(style, fieldName, label, inoutValue.data());
	case 3:
		return MkGui::drawFloat3Property(style, fieldName, label, inoutValue.data());
	case 4:
		return MkGui::drawFloat4Property(style, fieldName, label, inoutValue.data());
	default:
		return false;
	}
}

const std::string& getFloatTypeComboItems()
{
	static const std::string k_items= ShaderValueTypeUtils::toString(eShaderValueType::float1) + '\0'
									  + ShaderValueTypeUtils::toString(eShaderValueType::float2) + '\0'
									  + ShaderValueTypeUtils::toString(eShaderValueType::float3) + '\0'
									  + ShaderValueTypeUtils::toString(eShaderValueType::float4) + '\0';
	return k_items;
}

int getFloatTypeComboIndex(eShaderValueType type)
{
	return ShaderValueTypeUtils::isFloatVector(type) ? (int)type - (int)eShaderValueType::float1 : -1;
}

eShaderValueType getFloatTypeFromComboIndex(int index) { return ShaderValueTypeUtils::makeFloatVector(index + 1); }
} // namespace ShaderNodeUtils
