#pragma once

#include "ShaderNode.h"
#include "MkShaderConstants.h"

class ShaderSemanticInputNodeConfig : public NodeConfig
{
public:
	ShaderSemanticInputNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string semantic;
};

// A uniform the renderer binds by semantic rather than by parameter name:
// screen size and position, camera position, the near and far planes
class ShaderSemanticInputNode : public ShaderNode
{
public:
	ShaderSemanticInputNode()= default;

	inline static const std::string k_nodeClassName= "ShaderSemanticInputNode";
	inline static const std::string k_valuePinName= "value";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline void setSemantic(eUniformSemantic semantic) { m_semantic= semantic; }
	inline eUniformSemantic getSemantic() const { return m_semantic; }
	eShaderValueType getValueType() const;

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;

	static eUniformSemantic semanticFromName(const std::string& name);

protected:
	eUniformSemantic m_semantic= eUniformSemantic::screenSize;
};

class ShaderSemanticInputNodeFactory : public TypedNodeFactory<ShaderSemanticInputNode, ShaderSemanticInputNodeConfig>
{
public:
	ShaderSemanticInputNodeFactory(eUniformSemantic semantic);

	virtual std::string getFactoryKey() const override;
	virtual std::string editorGetCategory() const override;
	virtual NodePtr allocateNode() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;

protected:
	eUniformSemantic m_semantic;
};
