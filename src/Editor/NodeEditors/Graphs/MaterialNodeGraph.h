#pragma once

#include "NodeGraph.h"
#include "MaterialCompiler/MaterialCompileResult.h"
#include "MaterialCompiler/MaterialDomain.h"

#include <string>

class IShaderWriter;
class MaterialOutputNode;
using MaterialOutputNodePtr= std::shared_ptr<MaterialOutputNode>;

// A material authored as a graph of shader nodes. The graph is never
// evaluated at render time: compile() turns it into shader source and a
// material config through a writer, and the written files load through the
// ordinary .mat path.
class MaterialNodeGraph : public NodeGraph
{
public:
	MaterialNodeGraph();

	inline static const std::string k_graphClassName= "MaterialNodeGraph";
	virtual std::string getClassName() const override { return k_graphClassName; }

	virtual bool loadFromConfig(const NodeGraphConfig& config) override;
	virtual void saveToConfig(NodeGraphConfig& config) const override;

	// -- Domain -----
	eMaterialDomain getDomain() const { return m_domain; }
	eMaterialVertexPreset getVertexPreset() const { return m_vertexPreset; }
	// Changing the domain resets the preset to the domain's default and
	// rebuilds the output node's pins
	void setDomain(eMaterialDomain domain);
	// A preset the domain does not allow is ignored
	void setVertexPreset(eMaterialVertexPreset preset);
	MulticastDelegate<void()> OnDomainChanged;

	MaterialOutputNodePtr getOutputNode() const;

	// -- Compilation -----
	MaterialCompileResult compile(const IShaderWriter& writer);
	// Errors from the last compile, for the editor's error overlay
	const std::vector<NodeEvaluationError>& getLastCompileErrors() const { return m_lastCompileErrors; }

	// -- Editor -----
	virtual void editorRenderGraphPropertySheet(const class NodeEditorState& editorState) override;

protected:
	eMaterialDomain m_domain= eMaterialDomain::compositor;
	eMaterialVertexPreset m_vertexPreset= eMaterialVertexPreset::P2T;
	std::vector<NodeEvaluationError> m_lastCompileErrors;

	friend class MaterialNodeGraphFactory;
};

class MaterialNodeGraphFactory : public TypedNodeGraphFactory<MaterialNodeGraph>
{
public:
	MaterialNodeGraphFactory()= default;

	virtual NodeGraphPtr initialCreateNodeGraph(class IEditorWindow* ownerWindow) const override;
	// Same as above with the domain chosen up front
	NodeGraphPtr initialCreateMaterialGraph(class IEditorWindow* ownerWindow, eMaterialDomain domain) const;
};
