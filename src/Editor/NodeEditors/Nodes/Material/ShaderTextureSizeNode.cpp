#include "ShaderTextureSizeNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

// -- ShaderTextureSizeNode -----
bool ShaderTextureSizeNode::compileNode(MaterialCompileContext& context)
{
	const IShaderWriter& writer= context.getWriter();

	const ShaderValue texture= context.input(k_texturePinName);
	if (!texture.isValid())
		return false;

	if (texture.type != eShaderValueType::texture2D)
	{
		context.error("Texture Size needs a texture", getShaderInputPin(k_texturePinName));
		return false;
	}

	context.setOutput(k_sizePinName, context.emitTemp(eShaderValueType::float2, writer.textureSize(texture.expr)));

	return true;
}

std::string ShaderTextureSizeNode::editorGetTitle() const { return locText("nodes.textureSizeTitle"); }

const char* ShaderTextureSizeNode::editorGetHeaderIcon() const { return ICON_FK_ARROWS_ALT; }

// -- ShaderTextureSizeNodeFactory -----
std::string ShaderTextureSizeNodeFactory::editorGetCategory() const { return locText("nodes.categoryTexture"); }

NodePtr ShaderTextureSizeNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderTextureSizeNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr texturePin=
		node->addShaderInputPin(ShaderTextureSizeNode::k_texturePinName, eShaderValueType::texture2D);
	ShaderValuePinPtr sizePin= node->addShaderOutputPin(ShaderTextureSizeNode::k_sizePinName, eShaderValueType::float2);

	autoConnectInputPin(editorState, texturePin);
	autoConnectOutputPin(editorState, sizePin);

	return node;
}
