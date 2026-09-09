#pragma once

#include "ShaderNode.h"

class ShaderParameterNodeConfig : public NodeConfig
{
public:
	ShaderParameterNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string parameterName;
	std::string valueType;
	ShaderValueDefault defaultValue= {};
	bool bIsColor= false;
	bool bIsTime= false;
};

// The create menu entries of the parameter node. Each fixes the float type,
// whether the default edits as a color, and whether the parameter is the
// conventional "time" float the preview drives.
enum class eShaderParameterVariant : int
{
	float1,
	float2,
	float3,
	float4,
	color,
	time
};

// A material parameter: a uniform the consumer binds by name, typed by its
// generic parameter semantic, with a default the preview feeds.
class ShaderParameterNode : public ShaderNode
{
public:
	ShaderParameterNode()= default;

	inline static const std::string k_nodeClassName= "ShaderParameterNode";
	inline static const std::string k_valuePinName= "value";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	void setVariant(eShaderParameterVariant variant);
	inline eShaderValueType getValueType() const { return m_valueType; }
	inline bool isColor() const { return m_bIsColor; }
	inline bool isTime() const { return m_bIsTime; }
	inline const std::string& getParameterName() const { return m_parameterName; }
	// Parameter nodes sharing a name are one parameter. Taking a name another
	// node already carries adopts that node's default; a name carried by a node
	// of another type is refused and the name stays.
	bool setParameterName(const std::string& name);
	inline const ShaderValueDefault& getDefaultValue() const { return m_defaultValue; }
	// Writes the default to every parameter node of this name in the graph
	void setDefaultValue(const ShaderValueDefault& value);

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	std::string m_parameterName;
	eShaderValueType m_valueType= eShaderValueType::float1;
	bool m_bIsColor= false;
	bool m_bIsTime= false;
	ShaderValueDefault m_defaultValue= {};
};

class ShaderParameterNodeFactory : public TypedNodeFactory<ShaderParameterNode, ShaderParameterNodeConfig>
{
public:
	ShaderParameterNodeFactory(eShaderParameterVariant variant);

	virtual std::string getFactoryKey() const override;
	virtual std::string editorGetCategory() const override;
	virtual NodePtr allocateNode() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;

protected:
	eShaderParameterVariant m_variant;
};
