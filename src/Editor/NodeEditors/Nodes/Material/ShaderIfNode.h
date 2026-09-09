#pragma once

#include "ShaderNode.h"
#include "MaterialCompiler/IShaderWriter.h"

class ShaderIfNodeConfig : public NodeConfig
{
public:
	ShaderIfNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string comparison;
};

// Selects one of two values by comparing two others: whenTrue when the
// comparison of a and b holds, whenFalse otherwise. The ordering comparisons
// take scalars; equality also accepts vectors of one type.
class ShaderIfNode : public ShaderNode
{
public:
	ShaderIfNode()= default;

	inline static const std::string k_nodeClassName= "ShaderIfNode";
	inline static const std::string k_aPinName= "a";
	inline static const std::string k_bPinName= "b";
	inline static const std::string k_whenTruePinName= "whenTrue";
	inline static const std::string k_whenFalsePinName= "whenFalse";
	inline static const std::string k_resultPinName= "result";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline void setComparison(eShaderCompareOp comparison) { m_comparison= comparison; }
	inline eShaderCompareOp getComparison() const { return m_comparison; }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;

	// The persisted spelling of a comparison, and back
	static const std::string& comparisonToName(eShaderCompareOp comparison);
	static eShaderCompareOp comparisonFromName(const std::string& name);

protected:
	eShaderCompareOp m_comparison= eShaderCompareOp::less;
};

class ShaderIfNodeFactory : public TypedNodeFactory<ShaderIfNode, ShaderIfNodeConfig>
{
public:
	ShaderIfNodeFactory(eShaderCompareOp comparison);

	virtual std::string getFactoryKey() const override;
	virtual std::string editorGetCategory() const override;
	virtual NodePtr allocateNode() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;

protected:
	eShaderCompareOp m_comparison;
};
