#pragma once

#include "ShaderNode.h"

// -- Named reroute declaration -----

class ShaderRerouteDeclarationNodeConfig : public NodeConfig
{
public:
	ShaderRerouteDeclarationNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string name;
};

// Names a value so it can be picked up elsewhere on the page without a wire.
// Passes its input through its output as well, so it can sit inline on a chain.
class ShaderRerouteDeclarationNode : public ShaderNode
{
public:
	ShaderRerouteDeclarationNode()= default;

	inline static const std::string k_nodeClassName= "ShaderRerouteDeclarationNode";
	inline static const std::string k_valuePinName= "value";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline const std::string& getRerouteName() const { return m_name; }
	inline void setRerouteName(const std::string& name) { m_name= name; }

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;
	// Double-clicking the declaration drops a usage beside it
	virtual void editorOnDoubleClicked(const NodeEditorState& editorState) override;

protected:
	virtual ImVec4 editorGetHeaderColor() const override;

	std::string m_name;
};
using ShaderRerouteDeclarationNodePtr= std::shared_ptr<ShaderRerouteDeclarationNode>;

class ShaderRerouteDeclarationNodeFactory
	: public TypedNodeFactory<ShaderRerouteDeclarationNode, ShaderRerouteDeclarationNodeConfig>
{
public:
	ShaderRerouteDeclarationNodeFactory()= default;

	virtual std::string editorGetCategory() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};

// -- Named reroute usage -----

class ShaderRerouteUsageNodeConfig : public NodeConfig
{
public:
	ShaderRerouteUsageNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_node_id declarationId= -1;
};

// Reads a named reroute's value anywhere on the same page. It references the
// declaration by node id, so renaming the declaration renames every usage and
// deleting it leaves the usages reporting a missing declaration.
class ShaderRerouteUsageNode : public ShaderNode
{
public:
	ShaderRerouteUsageNode()= default;

	inline static const std::string k_nodeClassName= "ShaderRerouteUsageNode";
	inline static const std::string k_valuePinName= "value";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline void setDeclarationId(t_node_id declarationId) { m_declarationId= declarationId; }
	inline t_node_id getDeclarationId() const { return m_declarationId; }
	ShaderRerouteDeclarationNodePtr getDeclaration() const;

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;
	virtual void editorRenderPropertySheet(const NodeEditorState& editorState) override;

protected:
	virtual ImVec4 editorGetHeaderColor() const override;

	t_node_id m_declarationId= -1;
};
using ShaderRerouteUsageNodePtr= std::shared_ptr<ShaderRerouteUsageNode>;

class ShaderRerouteUsageNodeFactory : public TypedNodeFactory<ShaderRerouteUsageNode, ShaderRerouteUsageNodeConfig>
{
public:
	ShaderRerouteUsageNodeFactory()= default;

	virtual std::string editorGetCategory() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};
