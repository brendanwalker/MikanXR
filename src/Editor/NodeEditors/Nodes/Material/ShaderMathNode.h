#pragma once

#include "ShaderNode.h"
#include "MaterialCompiler/ShaderMathOpTable.h"

class ShaderMathNodeConfig : public NodeConfig
{
public:
	ShaderMathNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string op;
};

// One row of the math op table. A single class serves every op: the table
// names the inputs, the type rule that fixes the result type, and how the
// expression is spelled through the writer.
class ShaderMathNode : public ShaderNode
{
public:
	ShaderMathNode()= default;

	inline static const std::string k_nodeClassName= "ShaderMathNode";
	inline static const std::string k_resultPinName= "result";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline void setOp(eShaderMathOp op) { m_op= op; }
	inline eShaderMathOp getOp() const { return m_op; }
	const ShaderMathOpInfo& getOpInfo() const { return ShaderMathOpTable::getOpInfo(m_op); }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;

protected:
	eShaderMathOp m_op= eShaderMathOp::add;
};

class ShaderMathNodeFactory : public TypedNodeFactory<ShaderMathNode, ShaderMathNodeConfig>
{
public:
	ShaderMathNodeFactory(eShaderMathOp op);

	virtual std::string getFactoryKey() const override;
	virtual std::string editorGetCategory() const override;
	virtual NodePtr allocateNode() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;

protected:
	eShaderMathOp m_op;
};
