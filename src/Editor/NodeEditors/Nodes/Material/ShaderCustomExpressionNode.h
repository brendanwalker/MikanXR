#pragma once

#include "ShaderNode.h"

#include <vector>

// One declared input of a custom expression, as persisted
struct ShaderCustomExpressionInputConfig
{
	std::string name;
	std::string valueType;
};

class ShaderCustomExpressionNodeConfig : public NodeConfig
{
public:
	ShaderCustomExpressionNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string code;
	std::vector<ShaderCustomExpressionInputConfig> inputs;
	std::string outputType;
};

// One declared input of a custom expression: the parameter name the body
// refers to and the type the connected value is coerced to
struct ShaderCustomExpressionInput
{
	std::string name;
	eShaderValueType type= eShaderValueType::float1;
};

// A hand-written function body in the writer's language. The declared inputs
// become the function parameters and one dynamic input pin each, the body must
// return a value of the output type. The text is never translated to another
// language, so the node marks the material as GLSL only.
class ShaderCustomExpressionNode : public ShaderNode
{
public:
	ShaderCustomExpressionNode()= default;

	inline static const std::string k_nodeClassName= "ShaderCustomExpressionNode";
	inline static const std::string k_resultPinName= "result";
	inline static const size_t k_codeBufferSize= 4096;
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline const std::string& getCode() const { return m_code; }
	void setCode(const std::string& code);
	inline const std::vector<ShaderCustomExpressionInput>& getInputs() const { return m_inputs; }
	inline eShaderValueType getOutputType() const { return m_outputType; }

	// Retype the result pin, dropping links the new type cannot carry
	void setOutputType(eShaderValueType outputType);
	// Add a declared input with a fresh unique name and its pin
	void addInput();
	void removeInput(size_t index);
	// Ignored when the name is not an identifier or another input already uses it
	void renameInput(size_t index, const std::string& name);
	void setInputType(size_t index, eShaderValueType type);

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual bool editorShowsPortabilityWarning() const override { return true; }
	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	// Make the dynamic input pins match the declared inputs: pins without a
	// declaration go, missing ones are added, the rest keep their links and
	// follow their declared type
	void rebuildInputPins();
	bool hasInputNamed(const std::string& name) const;
	std::string makeUniqueInputName() const;
	// Retype a pin, deleting links that can no longer convert
	void retypePin(ShaderValuePinPtr pin, eShaderValueType type);

	std::string m_code;
	std::vector<ShaderCustomExpressionInput> m_inputs;
	eShaderValueType m_outputType= eShaderValueType::float4;

	// Editor scratch for the multiline body editor, synced from and to m_code
	char m_codeBuffer[k_codeBufferSize]= {};
};

class ShaderCustomExpressionNodeFactory
	: public TypedNodeFactory<ShaderCustomExpressionNode, ShaderCustomExpressionNodeConfig>
{
public:
	ShaderCustomExpressionNodeFactory()= default;

	virtual std::string editorGetCategory() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};
