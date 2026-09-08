#pragma once

#include "ShaderNode.h"

// Concatenates two float vectors into one wider vector: a float3 and a float1
// make a float4. The result never exceeds four components.
class ShaderAppendNode : public ShaderNode
{
public:
	ShaderAppendNode()= default;

	inline static const std::string k_nodeClassName= "ShaderAppendNode";
	inline static const std::string k_aPinName= "a";
	inline static const std::string k_bPinName= "b";
	inline static const std::string k_resultPinName= "result";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
};

class ShaderAppendNodeFactory : public TypedNodeFactory<ShaderAppendNode, NodeConfig>
{
public:
	ShaderAppendNodeFactory()= default;

	virtual std::string editorGetCategory() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};
