#pragma once

#include "ShaderNode.h"

// The pixel size of a texture, so a shader can address texels without a
// resolution parameter the consumer has to feed
class ShaderTextureSizeNode : public ShaderNode
{
public:
	ShaderTextureSizeNode()= default;

	inline static const std::string k_nodeClassName= "ShaderTextureSizeNode";
	inline static const std::string k_texturePinName= "texture";
	inline static const std::string k_sizePinName= "size";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
};

class ShaderTextureSizeNodeFactory : public TypedNodeFactory<ShaderTextureSizeNode, NodeConfig>
{
public:
	ShaderTextureSizeNodeFactory()= default;

	virtual std::string editorGetCategory() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};
