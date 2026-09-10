#include "ShaderTextureSampleNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

// -- ShaderTextureSampleNode -----
bool ShaderTextureSampleNode::compileNode(MaterialCompileContext& context)
{
	const IShaderWriter& writer= context.getWriter();

	const ShaderValue texture= context.input(k_texturePinName);
	if (!texture.isValid())
		return false;

	if (texture.type != eShaderValueType::texture2D)
	{
		context.error("Texture Sample needs a texture", getShaderInputPin(k_texturePinName));
		return false;
	}

	// An unconnected uv samples at the mesh texture coordinate rather than the pin's literal
	ShaderValuePinPtr uvPin= getShaderInputPin(k_uvPinName);
	ShaderValue uv= (uvPin && uvPin->hasAnyConnectedLinks()) ? context.input(uvPin)
															 : context.vertexAttribute(eVertexSemantic::texCoord);
	if (!uv.isValid())
		return false;

	uv= context.coerce(uv, eShaderValueType::float2);
	if (!uv.isValid())
		return false;

	const ShaderValue sample= context.emitTemp(eShaderValueType::float4, writer.sampleTexture(texture.expr, uv.expr));
	if (!sample.isValid())
		return false;

	context.setOutput(k_rgbaPinName, sample);
	context.setOutput(k_rPinName, {writer.swizzle(sample.expr, "x"), eShaderValueType::float1});
	context.setOutput(k_gPinName, {writer.swizzle(sample.expr, "y"), eShaderValueType::float1});
	context.setOutput(k_bPinName, {writer.swizzle(sample.expr, "z"), eShaderValueType::float1});
	context.setOutput(k_aPinName, {writer.swizzle(sample.expr, "w"), eShaderValueType::float1});

	return true;
}

std::string ShaderTextureSampleNode::editorGetTitle() const { return locText("nodes.textureSampleTitle"); }

const char* ShaderTextureSampleNode::editorGetHeaderIcon() const { return ICON_FK_EYE; }

// -- ShaderTextureSampleNodeFactory -----
std::string ShaderTextureSampleNodeFactory::editorGetCategory() const { return locText("nodes.categoryTexture"); }

NodePtr ShaderTextureSampleNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderTextureSampleNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr texturePin=
		node->addShaderInputPin(ShaderTextureSampleNode::k_texturePinName, eShaderValueType::texture2D);
	node->addShaderInputPin(ShaderTextureSampleNode::k_uvPinName, eShaderValueType::float2, {0.f, 0.f, 0.f, 0.f});

	ShaderValuePinPtr rgbaPin=
		node->addShaderOutputPin(ShaderTextureSampleNode::k_rgbaPinName, eShaderValueType::float4);
	node->addShaderOutputPin(ShaderTextureSampleNode::k_rPinName, eShaderValueType::float1);
	node->addShaderOutputPin(ShaderTextureSampleNode::k_gPinName, eShaderValueType::float1);
	node->addShaderOutputPin(ShaderTextureSampleNode::k_bPinName, eShaderValueType::float1);
	node->addShaderOutputPin(ShaderTextureSampleNode::k_aPinName, eShaderValueType::float1);

	autoConnectInputPin(editorState, texturePin);
	autoConnectOutputPin(editorState, rgbaPin);

	return node;
}
