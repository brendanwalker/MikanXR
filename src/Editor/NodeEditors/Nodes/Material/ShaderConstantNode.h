#pragma once

#include "ShaderNode.h"

class ShaderConstantNodeConfig : public NodeConfig
{
public:
	ShaderConstantNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string valueType;
	ShaderValueDefault value= {};
	bool bIsColor= false;
};

// A literal of one float type. One class serves every width plus the color
// variant, which is a float4 edited through a color picker.
class ShaderConstantNode : public ShaderNode
{
public:
	ShaderConstantNode()= default;

	inline static const std::string k_nodeClassName= "ShaderConstantNode";
	inline static const std::string k_valuePinName= "value";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	void setVariant(eShaderValueType valueType, bool bIsColor);
	inline eShaderValueType getValueType() const { return m_valueType; }
	inline bool isColor() const { return m_bIsColor; }
	inline const ShaderValueDefault& getValue() const { return m_value; }
	inline void setValue(const ShaderValueDefault& value) { m_value= value; }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
	virtual void editorRenderNode(const NodeEditorState& editorState) override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual void editorComputeNodeDimensions(NodeDimensions& outDims) const override;
	std::string editorGetValueText() const;

	eShaderValueType m_valueType= eShaderValueType::float1;
	bool m_bIsColor= false;
	ShaderValueDefault m_value= {};
};

class ShaderConstantNodeFactory : public TypedNodeFactory<ShaderConstantNode, ShaderConstantNodeConfig>
{
public:
	ShaderConstantNodeFactory(eShaderValueType valueType, bool bIsColor);

	virtual std::string getFactoryKey() const override;
	virtual std::string editorGetCategory() const override;
	virtual NodePtr allocateNode() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;

protected:
	eShaderValueType m_valueType;
	bool m_bIsColor;
};
