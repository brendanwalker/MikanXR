#pragma once

#include "ShaderNode.h"

// Samples a texture2D at a uv, falling back to the mesh texture coordinate
// when the uv pin is unconnected. Exposes the sample as rgba plus each channel.
class ShaderTextureSampleNode : public ShaderNode
{
public:
	ShaderTextureSampleNode()= default;

	inline static const std::string k_nodeClassName= "ShaderTextureSampleNode";
	inline static const std::string k_texturePinName= "texture";
	inline static const std::string k_uvPinName= "uv";
	inline static const std::string k_rgbaPinName= "rgba";
	inline static const std::string k_rPinName= "r";
	inline static const std::string k_gPinName= "g";
	inline static const std::string k_bPinName= "b";
	inline static const std::string k_aPinName= "a";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
};

class ShaderTextureSampleNodeFactory : public TypedNodeFactory<ShaderTextureSampleNode, NodeConfig>
{
public:
	ShaderTextureSampleNodeFactory()= default;

	virtual std::string editorGetCategory() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};
