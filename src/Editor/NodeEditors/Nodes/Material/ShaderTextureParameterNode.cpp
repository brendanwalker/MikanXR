#include "ShaderTextureParameterNode.h"
#include "ShaderNodeUtils.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "PathUtils.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

#include "imgui.h"
#include "tinyfiledialogs.h"

#include <algorithm>
#include <cstring>

namespace
{
const char* k_shaderTextureFilterPatterns[]= {"*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tga"};
const int k_shaderTextureFilterPatternCount= 5;
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
			m_parameterName= nameBuffer;
		}

		// The path field carries its own browse button
		if (MkGui::drawFilePathProperty(propertyStyle, "shaderTextureParameterDefault", locText("nodes.defaultTexture"),
										m_defaultTexturePath))
		{
			const char* picked=
				tinyfd_openFileDialog(locText("assets.loadTextureDialogTitle"), m_defaultTexturePath.c_str(),
									  k_shaderTextureFilterPatternCount, k_shaderTextureFilterPatterns,
									  locText("assets.imageFilterDescription"), 0);

			if (picked != nullptr && picked[0] != '\0')
			{
				m_defaultTexturePath= PathUtils::makeStoredProjectPath(picked);
			}
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
