#include "ShaderTextureParameterNode.h"
#include "ShaderNodeUtils.h"
#include "ShaderParameterNode.h"
#include "AssetPropertyGui.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "TextureAssetReference.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

#include "imgui.h"

#include <algorithm>
#include <cstring>

namespace
{
// The node keeps a texture path rather than an asset reference, so the property
// row borrows a factory for the type's glyph and its accepted file kinds
std::shared_ptr<TextureAssetReferenceFactory> getTextureAssetFactory()
{
	static std::shared_ptr<TextureAssetReferenceFactory> factory=
		AssetReferenceFactory::createFactory<TextureAssetReferenceFactory>();

	return factory;
}
} // namespace

// -- ShaderTextureParameterNodeConfig -----
configuru::Config ShaderTextureParameterNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["parameter_name"]= parameterName;
	pt["default_texture_path"]= defaultTexturePath;

	return pt;
}

void ShaderTextureParameterNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	parameterName= pt.get_or<std::string>("parameter_name", "");
	defaultTexturePath= pt.get_or<std::string>("default_texture_path", "");
}

// -- ShaderTextureParameterNode -----
bool ShaderTextureParameterNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderTextureParameterNodeConfig>(nodeConfig);

		m_parameterName= config->parameterName;
		m_defaultTexturePath= config->defaultTexturePath;

		return true;
	}

	return false;
}

void ShaderTextureParameterNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderTextureParameterNodeConfig>(nodeConfig);

	config->parameterName= m_parameterName;
	config->defaultTexturePath= m_defaultTexturePath;

	Node::saveToConfig(nodeConfig);
}

bool ShaderTextureParameterNode::setParameterName(const std::string& name)
{
	if (name == m_parameterName)
		return true;

	if (NodeGraphPtr graph= getOwnerGraph())
	{
		for (const auto& [nodeId, node] : graph->getNodesMap())
		{
			if (node.get() == this)
				continue;

			if (auto sibling= std::dynamic_pointer_cast<ShaderTextureParameterNode>(node))
			{
				if (sibling->getParameterName() == name)
				{
					m_defaultTexturePath= sibling->getDefaultTexturePath();
					break;
				}
			}
			else if (auto floatSibling= std::dynamic_pointer_cast<ShaderParameterNode>(node))
			{
				if (floatSibling->getParameterName() == name)
					return false;
			}
		}
	}

	m_parameterName= name;

	return true;
}

void ShaderTextureParameterNode::setDefaultTexturePath(const std::string& path)
{
	m_defaultTexturePath= path;

	if (NodeGraphPtr graph= getOwnerGraph())
	{
		for (const auto& [nodeId, node] : graph->getNodesMap())
		{
			auto sibling= std::dynamic_pointer_cast<ShaderTextureParameterNode>(node);
			if (sibling && sibling.get() != this && sibling->getParameterName() == m_parameterName)
			{
				sibling->m_defaultTexturePath= path;
			}
		}
	}
}

bool ShaderTextureParameterNode::compileNode(MaterialCompileContext& context)
{
	if (!ShaderNodeUtils::isValidIdentifier(m_parameterName))
	{
		context.error("Parameter name is not a valid identifier");
		return false;
	}

	const ShaderValue value=
		context.uniform(m_parameterName, eShaderValueType::texture2D, eUniformSemantic::textureParam);
	if (!value.isValid())
		return false;

	MaterialParameterDefault parameterDefault;
	parameterDefault.name= m_parameterName;
	parameterDefault.type= eShaderValueType::texture2D;
	parameterDefault.textureAssetPath= m_defaultTexturePath;
	context.addParameterDefault(parameterDefault);

	context.setOutput(k_texturePinName, value);

	return true;
}

std::string ShaderTextureParameterNode::editorGetTitle() const
{
	return !m_parameterName.empty() ? m_parameterName : locText("nodes.textureParameterTitle");
}

const char* ShaderTextureParameterNode::editorGetHeaderIcon() const { return ICON_FK_PICTURE_O; }

void ShaderTextureParameterNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.textureParameterHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		char nameBuffer[128];
		strncpy_s(nameBuffer, sizeof(nameBuffer), m_parameterName.c_str(), _TRUNCATE);
		if (MkGui::drawStringProperty(propertyStyle, "shaderTextureParameterName", locText("nodes.parameterName"),
									  nameBuffer, sizeof(nameBuffer)))
		{
			setParameterName(nameBuffer);
		}

		// The default texture is set by dropping a texture tile on the row
		std::string newTexturePath;
		if (AssetPropertyGui::drawAssetReferenceProperty(propertyStyle, "shaderTextureParameterDefault",
														 locText("nodes.defaultTexture"), *getTextureAssetFactory(),
														 m_defaultTexturePath, newTexturePath))
		{
			setDefaultTexturePath(newTexturePath);
		}
	}
}

// -- ShaderTextureParameterNodeFactory -----
std::string ShaderTextureParameterNodeFactory::editorGetCategory() const { return locText("nodes.categoryParameters"); }

NodePtr ShaderTextureParameterNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderTextureParameterNode>(NodeFactory::createNode(editorState));

	node->setParameterName("texture");

	ShaderValuePinPtr outputPin=
		node->addShaderOutputPin(ShaderTextureParameterNode::k_texturePinName, eShaderValueType::texture2D);

	autoConnectOutputPin(editorState, outputPin);

	return node;
}
