#pragma once

#include "ShaderNode.h"

class ShaderTextureParameterNodeConfig : public NodeConfig
{
public:
	ShaderTextureParameterNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string parameterName;
	std::string defaultTexturePath;
};

// A sampler parameter the consumer binds by name, with an optional default
// texture asset the preview loads
class ShaderTextureParameterNode : public ShaderNode
{
public:
	ShaderTextureParameterNode()= default;

	inline static const std::string k_nodeClassName= "ShaderTextureParameterNode";
	inline static const std::string k_texturePinName= "texture";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline const std::string& getParameterName() const { return m_parameterName; }
	// Texture parameter nodes sharing a name are one parameter. Taking a name
	// another texture parameter carries adopts its default texture; a name a
	// float parameter carries is refused and the name stays.
	bool setParameterName(const std::string& name);
	inline const std::string& getDefaultTexturePath() const { return m_defaultTexturePath; }
	// Writes the default texture to every texture parameter node of this name in the graph
	void setDefaultTexturePath(const std::string& path);

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	std::string m_parameterName;
	std::string m_defaultTexturePath;
};

class ShaderTextureParameterNodeFactory
	: public TypedNodeFactory<ShaderTextureParameterNode, ShaderTextureParameterNodeConfig>
{
public:
	ShaderTextureParameterNodeFactory()= default;

	virtual std::string editorGetCategory() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};
