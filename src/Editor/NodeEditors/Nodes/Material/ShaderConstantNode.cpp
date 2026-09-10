#include "ShaderConstantNode.h"
#include "ShaderNodeUtils.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkCanvasScopedNode.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

#include "imgui.h"

#include <algorithm>

// -- ShaderConstantNodeConfig -----
configuru::Config ShaderConstantNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["value_type"]= valueType;
	writeStdArray<float, 4>(pt, "value", value);
	pt["is_color"]= bIsColor;

	return pt;
}

void ShaderConstantNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	valueType= pt.get_or<std::string>("value_type", ShaderValueTypeUtils::toString(eShaderValueType::float1));
	if (pt.has_key("value"))
	{
		readStdArray<float, 4>(pt, "value", value);
	}
	bIsColor= pt.get_or<bool>("is_color", false);
}

// -- ShaderConstantNode -----
bool ShaderConstantNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderConstantNodeConfig>(nodeConfig);

		eShaderValueType valueType= ShaderValueTypeUtils::fromString(config->valueType);
		if (!ShaderValueTypeUtils::isFloatVector(valueType))
		{
			valueType= eShaderValueType::float1;
		}

		setVariant(valueType, config->bIsColor);
		m_value= config->value;

		return true;
	}

	return false;
}

void ShaderConstantNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderConstantNodeConfig>(nodeConfig);

	config->valueType= ShaderValueTypeUtils::toString(m_valueType);
	config->value= m_value;
	config->bIsColor= m_bIsColor;

	Node::saveToConfig(nodeConfig);
}

void ShaderConstantNode::setVariant(eShaderValueType valueType, bool bIsColor)
{
	m_valueType= bIsColor ? eShaderValueType::float4 : valueType;
	m_bIsColor= bIsColor;
}

bool ShaderConstantNode::compileNode(MaterialCompileContext& context)
{
	const IShaderWriter& writer= context.getWriter();

	context.setOutput(k_valuePinName, {writer.literal(m_valueType, m_value), m_valueType});

	return true;
}

std::string ShaderConstantNode::editorGetTitle() const
{
	if (m_bIsColor)
		return locText("nodes.colorConstantTitle");

	switch (m_valueType)
	{
	case eShaderValueType::float2:
		return locText("nodes.constant2Title");
	case eShaderValueType::float3:
		return locText("nodes.constant3Title");
	case eShaderValueType::float4:
		return locText("nodes.constant4Title");
	default:
		return locText("nodes.constantTitle");
	}
}

const char* ShaderConstantNode::editorGetHeaderIcon() const { return m_bIsColor ? ICON_FK_TINT : ICON_FK_HASHTAG; }

std::string ShaderConstantNode::editorGetValueText() const
{
	return ShaderNodeUtils::formatValueText(m_valueType, m_value);
}

void ShaderConstantNode::editorRenderNode(const NodeEditorState& editorState)
{
	MkCanvasScopedNode scopedNode(m_id, editorGetHeaderColor());

	editorRenderTitle(scopedNode);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));

	// The value reads on the canvas without opening the property sheet
	if (m_bIsColor)
	{
		const ImVec4 color(m_value[0], m_value[1], m_value[2], m_value[3]);
		ImGui::ColorButton("##constantColor", color, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_AlphaPreview);
	}
	else
	{
		ImGui::TextUnformatted(editorGetValueText().c_str());
	}

	editorRenderOutputPins(editorState);

	ImGui::Dummy(ImVec2(1.0f, 0.5f));
}

void ShaderConstantNode::editorComputeNodeDimensions(NodeDimensions& outDims) const
{
	ShaderNode::editorComputeNodeDimensions(outDims);

	if (!m_bIsColor)
	{
		const float valueWidth= ImGui::CalcTextSize(editorGetValueText().c_str()).x;
		outDims.totalNodeWidth= std::max(outDims.totalNodeWidth, valueWidth + outDims.outputColomnWidth);
	}
}

void ShaderConstantNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.shaderConstantHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		ShaderNodeUtils::drawValueProperty(propertyStyle, "shaderConstantValue", locText("nodes.value"), m_valueType,
										   m_bIsColor, m_value);
	}
}

// -- ShaderConstantNodeFactory -----
ShaderConstantNodeFactory::ShaderConstantNodeFactory(eShaderValueType valueType, bool bIsColor)
	: m_valueType(valueType)
	, m_bIsColor(bIsColor)
{
}

std::string ShaderConstantNodeFactory::getFactoryKey() const
{
	return ShaderConstantNode::k_nodeClassName + ":"
		   + (m_bIsColor ? std::string("color") : ShaderValueTypeUtils::toString(m_valueType));
}

std::string ShaderConstantNodeFactory::editorGetCategory() const { return locText("nodes.categoryConstants"); }

NodePtr ShaderConstantNodeFactory::allocateNode() const
{
	auto node= std::make_shared<ShaderConstantNode>();
	node->setVariant(m_valueType, m_bIsColor);
	if (m_bIsColor)
	{
		node->setValue({1.f, 1.f, 1.f, 1.f});
	}

	return node;
}

NodePtr ShaderConstantNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderConstantNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr outputPin= node->addShaderOutputPin(ShaderConstantNode::k_valuePinName, node->getValueType());

	autoConnectOutputPin(editorState, outputPin);

	return node;
}
