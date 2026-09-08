#pragma once

#include "IShaderWriter.h"
#include "MaterialCompileResult.h"
#include "MaterialDomain.h"
#include "NodeFwd.h"
#include "ShaderValueType.h"

#include <filesystem>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

class ShaderValuePin;
using ShaderValuePinPtr= std::shared_ptr<ShaderValuePin>;
class ShaderNode;
using ShaderNodePtr= std::shared_ptr<ShaderNode>;
class MaterialNodeGraph;
using MaterialNodeGraphPtr= std::shared_ptr<MaterialNodeGraph>;

// A symbolic value flowing along a link: an expression in the writer's language plus its type
struct ShaderValue
{
	std::string expr;
	eShaderValueType type= eShaderValueType::INVALID;

	bool isValid() const { return type != eShaderValueType::INVALID && !expr.empty(); }
};

// What a node sees while it compiles. Inputs are resolved on demand by walking
// the link to the upstream node and compiling it once per stage; declarations
// are deduplicated by name; errors are attributed to the node being compiled.
class MaterialCompileContext
{
public:
	MaterialCompileContext(MaterialNodeGraphPtr graph, const IShaderWriter& writer, MaterialCompileResult& result);

	eShaderStage getStage() const { return m_stage; }
	eMaterialDomain getDomain() const;
	eMaterialVertexPreset getVertexPreset() const;
	const IShaderWriter& getWriter() const { return m_writer; }
	ShaderNodePtr getCurrentNode() const;

	// -- Inputs -----
	// Compile whatever feeds the pin. Unconnected pins yield their default
	// literal (a wildcard pin's default is a float1). Errors yield an invalid value.
	ShaderValue input(ShaderValuePinPtr pin);
	// An input pin of the current node by name
	ShaderValue input(const std::string& pinName);
	// Broadcast a float1 to the target type, pass equal types through, error otherwise
	ShaderValue coerce(const ShaderValue& value, eShaderValueType targetType);

	// -- Emission -----
	// Declare a temporary in the current stage's main and return it as a value
	ShaderValue emitTemp(eShaderValueType type, const std::string& expr);
	// Declare a uniform once per program. The same name with a different type is an error.
	ShaderValue uniform(const std::string& name, eShaderValueType type, eUniformSemantic semantic);
	// The preset attribute with this semantic. In the vertex stage the attribute
	// itself; in the fragment stage a varying assigned from it. Missing from the preset is an error.
	ShaderValue vertexAttribute(eVertexSemantic semantic);
	// Window-space fragment position, an error outside the fragment stage
	ShaderValue fragmentCoord();
	// Declare a function once per stage and return a call to it
	ShaderValue callFunction(eShaderValueType returnType, const std::string& name,
							 const std::vector<ShaderFunctionParam>& params, const std::string& body,
							 const std::vector<ShaderValue>& args);

	// -- Outputs -----
	void setOutput(ShaderValuePinPtr pin, const ShaderValue& value);
	void setOutput(const std::string& pinName, const ShaderValue& value);

	// -- Diagnostics -----
	void error(const std::string& message, NodePinPtr pin= NodePinPtr());
	void setGlslOnly();
	void addParameterDefault(const MaterialParameterDefault& parameterDefault);

	// -- Driver (used by MaterialCompiler) -----
	// Compile the output node's pin in the given stage and return its value
	ShaderValue compileRoot(ShaderValuePinPtr outputPin, eShaderStage stage);
	// Assemble a stage's declarations and main into source text
	std::string buildStageSource(eShaderStage stage, const std::vector<std::string>& mainPrologue,
								 const std::vector<std::string>& mainEpilogue) const;
	// Ensure the matrix uniform used by the shape vertex stage is declared
	void declareMatrixUniform(const std::string& name, eUniformSemantic semantic);
	// Push resolved types back onto every pin the compile touched
	void applyResolvedTypes() const;
	// The uniforms declared so far, in declaration order, for the .mat semantic map
	const std::vector<std::string>& getUniformNames() const { return m_uniformOrder; }
	eUniformSemantic getUniformSemantic(const std::string& name) const;

private:
	struct StageBuilder
	{
		std::vector<std::string> attributeDeclarations;
		std::vector<std::string> uniformDeclarations;
		std::vector<std::string> varyingDeclarations;
		std::vector<std::string> functionDeclarations;
		std::vector<std::string> outputDeclarations;
		std::vector<std::string> varyingAssignments;
		std::vector<std::string> statements;
		std::set<std::string> declaredFunctions;
		std::set<std::string> declaredUniforms;
		std::set<eVertexSemantic> declaredAttributes;
		int nextTempIndex= 0;
	};

	struct UniformEntry
	{
		eShaderValueType type;
		eUniformSemantic semantic;
	};

	using NodeStageKey= std::pair<t_node_id, eShaderStage>;

	// Compile a node in the current stage (memoized), returning false on error
	bool compileNode(ShaderNodePtr node);
	ShaderValue lookupOutput(ShaderValuePinPtr pin);
	StageBuilder& stage() { return m_stages[(int)m_stage]; }
	const StageBuilder& stage(eShaderStage inStage) const { return m_stages[(int)inStage]; }
	void ensureVertexInputDeclared(eShaderStage inStage, const MaterialVertexAttribute& attribute);

	MaterialNodeGraphPtr m_graph;
	const IShaderWriter& m_writer;
	MaterialCompileResult& m_result;

	eShaderStage m_stage= eShaderStage::fragment;
	StageBuilder m_stages[2];
	std::map<std::string, UniformEntry> m_uniforms;
	std::vector<std::string> m_uniformOrder;
	std::map<eVertexSemantic, std::string> m_varyings;

	std::map<NodeStageKey, std::map<t_node_pin_id, ShaderValue>> m_nodeOutputs;
	std::set<NodeStageKey> m_visiting;
	std::vector<ShaderNodePtr> m_nodeStack;
	std::map<t_node_pin_id, eShaderValueType> m_resolvedPinTypes;
};

class MaterialCompiler
{
public:
	// Compile the graph against a writer. Never touches the graph's owner
	// window or any GL resource, so it runs headless.
	static MaterialCompileResult compile(MaterialNodeGraphPtr graph, const IShaderWriter& writer);

	// Write <stem>.vert, <stem>.frag and <stem>.mat beside the graph file,
	// filling the result config's paths and material name. The .mat names the
	// graph file as its source.
	static bool writeOutputs(MaterialCompileResult& result, const std::filesystem::path& graphPath,
							 std::string& outError);

	static std::filesystem::path getMaterialPathForGraph(const std::filesystem::path& graphPath);
	static std::filesystem::path getVertexShaderPathForGraph(const std::filesystem::path& graphPath);
	static std::filesystem::path getFragmentShaderPathForGraph(const std::filesystem::path& graphPath);

	// Name of the matrix uniform the shape vertex stage projects through
	inline static const std::string k_mvpUniformName= "mvpMatrix";
};
