#pragma once

#include "ShaderNode.h"

class ShaderSwizzleNodeConfig : public NodeConfig
{
public:
	ShaderSwizzleNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string mask= "xyz";
};

// Reorders, drops or repeats the components of a float vector through a
// mask of one to four letters (xyzw or rgba). The result width is the mask
// length, so the same node splits a float4 or widens a float1.
class ShaderSwizzleNode : public ShaderNode
{
public:
	ShaderSwizzleNode()= default;

	inline static const std::string k_nodeClassName= "ShaderSwizzleNode";
	inline static const std::string k_valuePinName= "value";
	inline static const std::string k_resultPinName= "result";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline const std::string& getMask() const { return m_mask; }
	inline void setMask(const std::string& mask) { m_mask= mask; }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

	// True when every letter of the mask names a component the input type has
	static bool isMaskValidForType(const std::string& mask, eShaderValueType inputType);

protected:
	std::string m_mask= "xyz";
};

class ShaderSwizzleNodeFactory : public TypedNodeFactory<ShaderSwizzleNode, ShaderSwizzleNodeConfig>
{
public:
	ShaderSwizzleNodeFactory()= default;

	virtual std::string editorGetCategory() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};
