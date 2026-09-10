#include "ShaderNode.h"
#include "IconsForkAwesome.h"
#include "MkCanvasScopedNode.h"
#include "MkGuiScopedStyleVar.h"

#include "imgui.h"

#include <algorithm>

ShaderValuePinPtr ShaderNode::getShaderInputPin(const std::string& pinName) const
{
	for (NodePinPtr pin : m_pinsIn)
	{
		if (pin->getName() == pinName)
			return std::dynamic_pointer_cast<ShaderValuePin>(pin);
	}

	return ShaderValuePinPtr();
}

ShaderValuePinPtr ShaderNode::getShaderOutputPin(const std::string& pinName) const
{
	for (NodePinPtr pin : m_pinsOut)
	{
		if (pin->getName() == pinName)
			return std::dynamic_pointer_cast<ShaderValuePin>(pin);
	}

	return ShaderValuePinPtr();
}

ShaderValuePinPtr ShaderNode::addShaderInputPin(const std::string& pinName, eShaderValueType type,
												const ShaderValueDefault& defaultValue)
{
	ShaderValuePinPtr pin= addPin<ShaderValuePin>(pinName, eNodePinDirection::INPUT);
	pin->setDeclaredType(type);
	pin->setDefaultValue(defaultValue);

	return pin;
}

ShaderValuePinPtr ShaderNode::addShaderOutputPin(const std::string& pinName, eShaderValueType type)
{
	ShaderValuePinPtr pin= addPin<ShaderValuePin>(pinName, eNodePinDirection::OUTPUT);
	pin->setDeclaredType(type);

	return pin;
}

ImVec4 ShaderNode::editorGetHeaderColor() const
{
	return ImVec4(96.f / 255.f, 110.f / 255.f, 150.f / 255.f, 225.f / 255.f);
}

const char* ShaderNode::editorGetPortabilityWarningSuffix() { return "  " ICON_FK_EXCLAMATION_TRIANGLE; }

void ShaderNode::editorRenderTitle(MkCanvasScopedNode& scopedNode) const
{
	const std::string titleString= editorGetComposedTitle();

	scopedNode.beginHeader();
	{
		MkGuiScopedStyleVar styleVar;
		styleVar.push(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		ImGui::TextUnformatted(titleString.c_str());

		if (editorShowsPortabilityWarning())
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.f, 0.8f, 0.2f, 1.f), "%s", editorGetPortabilityWarningSuffix());
		}
	}
	scopedNode.endHeader();
}

void ShaderNode::editorComputeNodeDimensions(NodeDimensions& outDims) const
{
	Node::editorComputeNodeDimensions(outDims);

	if (editorShowsPortabilityWarning())
	{
		outDims.titleWidth+= ImGui::CalcTextSize(editorGetPortabilityWarningSuffix()).x;
		outDims.totalNodeWidth= std::max(outDims.totalNodeWidth, outDims.titleWidth);
	}
}
