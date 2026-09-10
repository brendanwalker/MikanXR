#include "ShaderSwizzleNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

#include <cstring>

namespace
{
const std::string k_swizzleXyzwLetters= "xyzw";
const std::string k_swizzleRgbaLetters= "rgba";
const size_t k_swizzleMaxMaskLength= 4;

// Component index of a mask letter within its letter set, npos when it is not a letter of the set
size_t getSwizzleComponentIndex(const std::string& letterSet, char letter) { return letterSet.find(letter); }
} // namespace

// -- ShaderSwizzleNodeConfig -----
configuru::Config ShaderSwizzleNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["mask"]= mask;

	return pt;
}

void ShaderSwizzleNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	mask= pt.get_or<std::string>("mask", "xyz");
}

// -- ShaderSwizzleNode -----
bool ShaderSwizzleNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderSwizzleNodeConfig>(nodeConfig);

		m_mask= config->mask;

		return true;
	}

	return false;
}

void ShaderSwizzleNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderSwizzleNodeConfig>(nodeConfig);

	config->mask= m_mask;

	Node::saveToConfig(nodeConfig);
}

bool ShaderSwizzleNode::isMaskValidForType(const std::string& mask, eShaderValueType inputType)
{
	if (mask.empty() || mask.size() > k_swizzleMaxMaskLength)
		return false;

	const int componentCount= ShaderValueTypeUtils::getComponentCount(inputType);
	if (componentCount <= 0)
		return false;

	// A mask draws all of its letters from one set: xyzw or rgba, never a mix
	const std::string& letterSet= (getSwizzleComponentIndex(k_swizzleRgbaLetters, mask[0]) != std::string::npos)
									  ? k_swizzleRgbaLetters
									  : k_swizzleXyzwLetters;
	for (char letter : mask)
	{
		const size_t componentIndex= getSwizzleComponentIndex(letterSet, letter);
		if (componentIndex == std::string::npos || (int)componentIndex >= componentCount)
			return false;
	}

	return true;
}

bool ShaderSwizzleNode::compileNode(MaterialCompileContext& context)
{
	const IShaderWriter& writer= context.getWriter();

	const ShaderValue value= context.input(k_valuePinName);
	if (!value.isValid())
		return false;

	if (!isMaskValidForType(m_mask, value.type))
	{
		context.error("Swizzle mask " + m_mask + " does not fit a " + ShaderValueTypeUtils::toString(value.type),
					  getShaderInputPin(k_valuePinName));
		return false;
	}

	// A scalar has no components to select from, so it widens first and the
	// mask then repeats its single component
	const std::string sourceExpr=
		(value.type == eShaderValueType::float1) ? writer.broadcast(value.expr, eShaderValueType::float4) : value.expr;

	const eShaderValueType resultType= ShaderValueTypeUtils::makeFloatVector((int)m_mask.size());
	context.setOutput(k_resultPinName, context.emitTemp(resultType, writer.swizzle(sourceExpr, m_mask)));

	return true;
}

std::string ShaderSwizzleNode::editorGetTitle() const { return locText("nodes.swizzleTitle"); }

const char* ShaderSwizzleNode::editorGetHeaderIcon() const { return ICON_FK_RANDOM; }

void ShaderSwizzleNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.shaderSwizzleHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		char maskBuffer[16];
		strncpy_s(maskBuffer, sizeof(maskBuffer), m_mask.c_str(), _TRUNCATE);
		if (MkGui::drawStringProperty(propertyStyle, "shaderSwizzleMask", locText("nodes.swizzleMask"), maskBuffer,
									  sizeof(maskBuffer)))
		{
			m_mask= maskBuffer;
		}
	}
}

// -- ShaderSwizzleNodeFactory -----
std::string ShaderSwizzleNodeFactory::editorGetCategory() const { return locText("nodes.categoryVector"); }

NodePtr ShaderSwizzleNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderSwizzleNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr valuePin= node->addShaderInputPin(ShaderSwizzleNode::k_valuePinName, eShaderValueType::wildcard);
	ShaderValuePinPtr resultPin=
		node->addShaderOutputPin(ShaderSwizzleNode::k_resultPinName, eShaderValueType::wildcard);

	autoConnectInputPin(editorState, valuePin);
	autoConnectOutputPin(editorState, resultPin);

	return node;
}
