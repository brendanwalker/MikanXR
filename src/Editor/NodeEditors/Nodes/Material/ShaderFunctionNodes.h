#pragma once

#include "LocText.h"
#include "ShaderNode.h"

#include <vector>

class MaterialFunctionPage;
using MaterialFunctionPagePtr= std::shared_ptr<MaterialFunctionPage>;

// -- Function input -----

class ShaderFunctionInputNodeConfig : public NodeConfig
{
public:
	ShaderFunctionInputNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string inputName;
};

// One declared input of the function page it sits on. The page creates and
// deletes these with the declaration; the node compiles to the parameter name.
class ShaderFunctionInputNode : public ShaderNode
{
public:
	ShaderFunctionInputNode()= default;

	inline static const std::string k_nodeClassName= "ShaderFunctionInputNode";
	inline static const std::string k_valuePinName= "value";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline void setInputName(const std::string& name) { m_inputName= name; }
	inline const std::string& getInputName() const { return m_inputName; }
	// Follows a rename or retype of the declaration
	void syncToDeclaration(const std::string& name, eShaderValueType type);

	virtual bool compileNode(MaterialCompileContext& context) override;

	// The declaration owns the node's lifetime
	virtual bool editorCanDelete() const override;
	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;

protected:
	MaterialFunctionPagePtr getFunctionPage() const;

	std::string m_inputName;
};
using ShaderFunctionInputNodePtr= std::shared_ptr<ShaderFunctionInputNode>;

class ShaderFunctionInputNodeFactory : public TypedNodeFactory<ShaderFunctionInputNode, ShaderFunctionInputNodeConfig>
{
public:
	ShaderFunctionInputNodeFactory()= default;

	// Created by the page with its declaration, never from the create menu
	virtual bool editorCanCreate() const override { return false; }
};

// -- Function output -----

// The root of a function page: its one input pin is the function's return value
class ShaderFunctionOutputNode : public ShaderNode
{
public:
	ShaderFunctionOutputNode()= default;

	inline static const std::string k_nodeClassName= "ShaderFunctionOutputNode";
	inline static const std::string k_valuePinName= "value";
	virtual std::string getClassName() const override { return k_nodeClassName; }

	// Compiled through its pin by the function call, never as a node
	virtual bool compileNode(MaterialCompileContext& context) override { return true; }

	ShaderValuePinPtr getValuePin() const { return getShaderInputPin(k_valuePinName); }
	// Follows a retype of the page's output
	void syncToOutputType(eShaderValueType type);

	virtual bool editorCanDelete() const override { return false; }
	virtual std::string editorGetTitle() const override { return locText("nodes.functionOutputTitle"); }
	virtual const char* editorGetHeaderIcon() const override;

protected:
	virtual ImVec4 editorGetHeaderColor() const override;
};
using ShaderFunctionOutputNodePtr= std::shared_ptr<ShaderFunctionOutputNode>;

class ShaderFunctionOutputNodeFactory : public TypedNodeFactory<ShaderFunctionOutputNode, NodeConfig>
{
public:
	ShaderFunctionOutputNodeFactory()= default;

	virtual bool editorCanCreate() const override { return false; }
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;
};

// -- Function call -----

class ShaderFunctionCallNodeConfig : public NodeConfig
{
public:
	ShaderFunctionCallNodeConfig()= default;

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	t_graph_page_id pageId= -1;
};

// Calls a function page: one dynamic input pin per declared input, one result
// pin of the output type. Follows the page's edits and goes with the page.
class ShaderFunctionCallNode : public ShaderNode
{
public:
	ShaderFunctionCallNode()= default;
	virtual ~ShaderFunctionCallNode();

	inline static const std::string k_nodeClassName= "ShaderFunctionCallNode";
	inline static const std::string k_resultPinName= "result";
	virtual std::string getClassName() const override { return k_nodeClassName; }
	virtual void setOwnerGraph(NodeGraphPtr ownerGraph) override;

	virtual bool loadFromConfig(NodeConfigConstPtr nodeConfig) override;
	virtual void saveToConfig(NodeConfigPtr nodeConfig) const override;

	inline void setFunctionPageId(t_graph_page_id pageId) { m_pageId= pageId; }
	inline t_graph_page_id getFunctionPageId() const { return m_pageId; }
	inline void setFunctionName(const std::string& name) { m_functionName= name; }
	MaterialFunctionPagePtr getFunctionPage() const;

	// Make the pins match the page's declaration, keeping same-named pins and their links
	void rebuildPins();

	virtual bool compileNode(MaterialCompileContext& context) override;

	virtual std::string editorGetTitle() const override;
	virtual const char* editorGetHeaderIcon() const override;

protected:
	void onGraphLoaded(bool success);
	void onPageModified(t_graph_page_id pageId);
	void onPageDeleted(t_graph_page_id pageId);
	void retypePin(ShaderValuePinPtr pin, eShaderValueType type);

	t_graph_page_id m_pageId= -1;
	// Cached for the title, so the create menu's default object reads right without a graph
	std::string m_functionName;
};
using ShaderFunctionCallNodePtr= std::shared_ptr<ShaderFunctionCallNode>;

// One factory per function page, registered by the material graph under
// ShaderFunctionCallNode:<pageId> while the page exists
class ShaderFunctionCallNodeFactory : public TypedNodeFactory<ShaderFunctionCallNode, ShaderFunctionCallNodeConfig>
{
public:
	ShaderFunctionCallNodeFactory(t_graph_page_id pageId, const std::string& functionName);

	static std::string makeFactoryKey(t_graph_page_id pageId);
	virtual std::string getFactoryKey() const override { return makeFactoryKey(m_pageId); }
	// The graph registers one factory with no page so the class loads; only page-bound ones create
	virtual bool editorCanCreate() const override { return m_pageId != -1; }
	virtual std::string editorGetCategory() const override;
	virtual NodePtr allocateNode() const override;
	virtual NodePtr createNode(const NodeEditorState& editorState) const override;

protected:
	t_graph_page_id m_pageId;
	std::string m_functionName;
};
