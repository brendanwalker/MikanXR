#include "ShaderParameterNode.h"
#include "ShaderNodeUtils.h"
#include "ShaderTextureParameterNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

#include "imgui.h"

#include <cstring>

namespace
{
const char* k_shaderParameterVariantNames[]= {"float", "float2", "float3", "float4", "color", "time"};
const char* k_shaderParameterDefaultNames[]= {"param", "param2", "param3", "param4", "color", "time"};

eShaderValueType getShaderParameterVariantType(eShaderParameterVariant variant)
{
	switch (variant)
	{
	case eShaderParameterVariant::float2:
		return eShaderValueType::float2;
	case eShaderParameterVariant::float3:
		return eShaderValueType::float3;
	case eShaderParameterVariant::float4:
	case eShaderParameterVariant::color:
		return eShaderValueType::float4;
	default:
		return eShaderValueType::float1;
	}
}
} // namespace

// -- ShaderParameterNodeConfig -----
configuru::Config ShaderParameterNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["parameter_name"]= parameterName;
	pt["value_type"]= valueType;
	writeStdArray<float, 4>(pt, "default_value", defaultValue);
	pt["is_color"]= bIsColor;
	pt["is_time"]= bIsTime;

	return pt;
}

void ShaderParameterNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	parameterName= pt.get_or<std::string>("parameter_name", "");
	valueType= pt.get_or<std::string>("value_type", ShaderValueTypeUtils::toString(eShaderValueType::float1));
	if (pt.has_key("default_value"))
	{
		readStdArray<float, 4>(pt, "default_value", defaultValue);
	}
	bIsColor= pt.get_or<bool>("is_color", false);
	bIsTime= pt.get_or<bool>("is_time", false);
}

// -- ShaderParameterNode -----
bool ShaderParameterNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderParameterNodeConfig>(nodeConfig);

		eShaderValueType valueType= ShaderValueTypeUtils::fromString(config->valueType);
		if (!ShaderValueTypeUtils::isFloatVector(valueType))
		{
			valueType= eShaderValueType::float1;
		}

		m_parameterName= config->parameterName;
		m_valueType= config->bIsColor ? eShaderValueType::float4 : valueType;
		m_bIsColor= config->bIsColor;
		m_bIsTime= config->bIsTime;
		m_defaultValue= config->defaultValue;

		return true;
	}

	return false;
}

void ShaderParameterNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderParameterNodeConfig>(nodeConfig);

	config->parameterName= m_parameterName;
	config->valueType= ShaderValueTypeUtils::toString(m_valueType);
	config->defaultValue= m_defaultValue;
	config->bIsColor= m_bIsColor;
	config->bIsTime= m_bIsTime;

	Node::saveToConfig(nodeConfig);
}

void ShaderParameterNode::setVariant(eShaderParameterVariant variant)
{
	m_valueType= getShaderParameterVariantType(variant);
	m_bIsColor= (variant == eShaderParameterVariant::color);
	m_bIsTime= (variant == eShaderParameterVariant::time);
	if (m_bIsColor)
	{
		m_defaultValue= {1.f, 1.f, 1.f, 1.f};
	}
}

bool ShaderParameterNode::setParameterName(const std::string& name)
{
	if (name == m_parameterName)
		return true;

	if (NodeGraphPtr graph= getOwnerGraph())
	{
		for (const auto& [nodeId, node] : graph->getNodesMap())
		{
			if (node.get() == this)
				continue;

			if (auto sibling= std::dynamic_pointer_cast<ShaderParameterNode>(node))
			{
				if (sibling->getParameterName() != name)
					continue;
				if (sibling->getValueType() != m_valueType)
					return false;

				m_defaultValue= sibling->getDefaultValue();
				m_bIsColor= sibling->isColor();
				break;
			}
			else if (auto textureSibling= std::dynamic_pointer_cast<ShaderTextureParameterNode>(node))
			{
				if (textureSibling->getParameterName() == name)
					return false;
			}
		}
	}

	m_parameterName= name;

	return true;
}

void ShaderParameterNode::setDefaultValue(const ShaderValueDefault& value)
{
	m_defaultValue= value;

	if (NodeGraphPtr graph= getOwnerGraph())
	{
		for (const auto& [nodeId, node] : graph->getNodesMap())
		{
			auto sibling= std::dynamic_pointer_cast<ShaderParameterNode>(node);
			if (sibling && sibling.get() != this && sibling->getParameterName() == m_parameterName
				&& sibling->getValueType() == m_valueType)
			{
				sibling->m_defaultValue= value;
			}
		}
	}
}

bool ShaderParameterNode::compileNode(MaterialCompileContext& context)
{
	if (!ShaderNodeUtils::isValidIdentifier(m_parameterName))
	{
		context.error("Parameter name is not a valid identifier");
		return false;
	}

	const ShaderValue value=
		context.uniform(m_parameterName, m_valueType, ShaderValueTypeUtils::getParameterSemantic(m_valueType));
	if (!value.isValid())
		return false;

	MaterialParameterDefault parameterDefault;
	parameterDefault.name= m_parameterName;
	parameterDefault.type= m_valueType;
	parameterDefault.value= m_defaultValue;
	context.addParameterDefault(parameterDefault);

	context.setOutput(k_valuePinName, value);

	return true;
}

std::string ShaderParameterNode::editorGetTitle() const
{
	if (!m_parameterName.empty())
		return m_parameterName;

	if (m_bIsTime)
		return locText("nodes.timeParameterTitle");

	if (m_bIsColor)
		return locText("nodes.colorParameterTitle");

	switch (m_valueType)
	{
	case eShaderValueType::float2:
		return locText("nodes.parameter2Title");
	case eShaderValueType::float3:
		return locText("nodes.parameter3Title");
	case eShaderValueType::float4:
		return locText("nodes.parameter4Title");
	default:
		return locText("nodes.parameterTitle");
	}
}

const char* ShaderParameterNode::editorGetHeaderIcon() const
{
	if (m_bIsTime)
		return ICON_FK_CLOCK_O;

	return m_bIsColor ? ICON_FK_TINT : ICON_FK_SLIDERS;
}

void ShaderParameterNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.shaderParameterHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		char nameBuffer[128];
		strncpy_s(nameBuffer, sizeof(nameBuffer), m_parameterName.c_str(), _TRUNCATE);
		if (MkGui::drawStringProperty(propertyStyle, "shaderParameterName", locText("nodes.parameterName"), nameBuffer,
									  sizeof(nameBuffer)))
		{
			setParameterName(nameBuffer);
		}

		// The edit lands on every node of this name
		if (ShaderNodeUtils::drawValueProperty(propertyStyle, "shaderParameterDefault", locText("nodes.defaultValue"),
											   m_valueType, m_bIsColor, m_defaultValue))
		{
			setDefaultValue(m_defaultValue);
		}
	}
}

// -- ShaderParameterNodeFactory -----
ShaderParameterNodeFactory::ShaderParameterNodeFactory(eShaderParameterVariant variant)
	: m_variant(variant)
{
}

std::string ShaderParameterNodeFactory::getFactoryKey() const
{
	return ShaderParameterNode::k_nodeClassName + ":" + k_shaderParameterVariantNames[(int)m_variant];
}

std::string ShaderParameterNodeFactory::editorGetCategory() const { return locText("nodes.categoryParameters"); }

NodePtr ShaderParameterNodeFactory::allocateNode() const
{
	auto node= std::make_shared<ShaderParameterNode>();
	node->setVariant(m_variant);

	return node;
}

NodePtr ShaderParameterNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderParameterNode>(NodeFactory::createNode(editorState));

	// The default object keeps an empty name so the create menu shows the variant title
	node->setParameterName(k_shaderParameterDefaultNames[(int)m_variant]);

	ShaderValuePinPtr outputPin= node->addShaderOutputPin(ShaderParameterNode::k_valuePinName, node->getValueType());

	autoConnectOutputPin(editorState, outputPin);

	return node;
}
