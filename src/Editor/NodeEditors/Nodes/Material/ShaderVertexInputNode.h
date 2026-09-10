#pragma once

#include "ShaderNode.h"
#include "MkVertexConstants.h"

class ShaderVertexInputNodeConfig : public NodeConfig
{
public:
	ShaderVertexInputNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string semantic;
};

// One attribute of the graph's vertex preset. In the fragment stage the
// compiler routes it through a varying, so the node reads the same in both.
class ShaderVertexInputNode : public ShaderNode
{
public:
	ShaderVertexInputNode()= default;

	inline static const std::string k_nodeClassName= "ShaderVertexInputNode";
	inline static const std::string k_valuePinName= "value";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline void setSemantic(eVertexSemantic semantic) { m_semantic= semantic; }
	inline eVertexSemantic getSemantic() const { return m_semantic; }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;

protected:
	eVertexSemantic m_semantic= eVertexSemantic::position;
};

class ShaderVertexInputNodeFactory : public TypedNodeFactory<ShaderVertexInputNode, ShaderVertexInputNodeConfig>
{
public:
	ShaderVertexInputNodeFactory(eVertexSemantic semantic);

	virtual std::string getFactoryKey() const override;
	virtual std::string editorGetCategory() const override;
	virtual NodePtr allocateNode() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;

protected:
	eVertexSemantic m_semantic;
};
