#include "MaterialCompiler.h"
#include "Logger.h"
#include "MkVertexConstants.h"
#include "Graphs/MaterialNodeGraph.h"
#include "Graphs/NodeGraph.h"
#include "Nodes/Material/MaterialOutputNode.h"
#include "Nodes/Material/ShaderNode.h"
#include "Pins/NodeLink.h"
#include "Pins/ShaderValuePin.h"

#include <fstream>

namespace
{
// The varying that carries an attribute into the fragment stage: aTexCoords becomes vTexCoords
std::string makeVaryingName(const std::string& attributeName)
{
	if (attributeName.size() > 1 && attributeName[0] == 'a')
	{
		return "v" + attributeName.substr(1);
	}

	return "v" + attributeName;
}

void appendLines(std::vector<std::string>& lines, const std::vector<std::string>& group)
{
	lines.insert(lines.end(), group.begin(), group.end());
}

bool writeTextFile(const std::filesystem::path& path, const std::string& text, std::string& outError)
{
	// Binary mode keeps the LF line endings the compiler emits
	std::ofstream file(path, std::ios::binary);
	if (!file)
	{
		outError= "Failed to open " + path.string() + " for writing";
		return false;
	}

	file << text;
	if (!file.good())
	{
		outError= "Failed to write " + path.string();
		return false;
	}

	return true;
}
} // namespace

// -- MaterialCompileContext -----
MaterialCompileContext::MaterialCompileContext(MaterialNodeGraphPtr graph, const IShaderWriter& writer,
											   MaterialCompileResult& result)
	: m_graph(graph)
	, m_writer(writer)
	, m_result(result)
{
	// The fixed interface of both stages: every preset attribute feeds the
	// vertex stage in location order, and the fragment stage writes one color.
	// Declaring them up front keeps the attribute order independent of which
	// node happens to ask for an attribute first.
	for (const MaterialVertexAttribute& attribute : MaterialDomainUtils::getPresetAttributes(getVertexPreset()))
	{
		ensureVertexInputDeclared(eShaderStage::vertex, attribute);
	}

	m_stages[(int)eShaderStage::fragment].outputDeclarations.push_back(m_writer.declareFragmentColorOutput());
}

eMaterialDomain MaterialCompileContext::getDomain() const
{
	return m_graph ? m_graph->getDomain() : eMaterialDomain::INVALID;
}

eMaterialVertexPreset MaterialCompileContext::getVertexPreset() const
{
	return m_graph ? m_graph->getVertexPreset() : eMaterialVertexPreset::INVALID;
}

ShaderNodePtr MaterialCompileContext::getCurrentNode() const
{
	return m_nodeStack.empty() ? ShaderNodePtr() : m_nodeStack.back();
}

// -- Inputs -----
ShaderValue MaterialCompileContext::input(ShaderValuePinPtr pin)
{
	if (!pin)
	{
		error("Input pin is missing");
		return ShaderValue();
	}

	NodePinPtr sourcePin= pin->getConnectedSourcePin();
	if (sourcePin)
	{
		auto shaderSourcePin= std::dynamic_pointer_cast<ShaderValuePin>(sourcePin);
		auto sourceNode= std::dynamic_pointer_cast<ShaderNode>(sourcePin->getOwnerNode());
		if (!shaderSourcePin || !sourceNode)
		{
			error("Pin is fed by a node that is not a shader node", pin);
			return ShaderValue();
		}

		// A failed compile has already reported why, so do not add a missing-value error on top
		if (!compileNode(sourceNode))
			return ShaderValue();

		ShaderValue value= lookupOutput(shaderSourcePin);
		if (!value.isValid())
		{
			error("Node produced no value for pin " + pin->getName(), pin);
			return ShaderValue();
		}

		m_resolvedPinTypes[pin->getId()]= value.type;
		m_resolvedPinTypes[shaderSourcePin->getId()]= value.type;

		return value;
	}

	// Unconnected: the pin's default literal
	const eShaderValueType declaredType= pin->getDeclaredType();
	if (ShaderValueTypeUtils::isFloatVector(declaredType))
	{
		m_resolvedPinTypes[pin->getId()]= declaredType;
		return {m_writer.literal(declaredType, pin->getDefaultValue()), declaredType};
	}

	if (declaredType == eShaderValueType::wildcard)
	{
		m_resolvedPinTypes[pin->getId()]= eShaderValueType::float1;
		return {m_writer.literal(eShaderValueType::float1, pin->getDefaultValue()), eShaderValueType::float1};
	}

	if (declaredType == eShaderValueType::texture2D)
	{
		error("Texture input " + pin->getName() + " needs a connection", pin);
		return ShaderValue();
	}

	error("Input " + pin->getName() + " has no usable type", pin);
	return ShaderValue();
}

ShaderValue MaterialCompileContext::input(const std::string& pinName)
{
	ShaderNodePtr node= getCurrentNode();
	ShaderValuePinPtr pin= node ? node->getShaderInputPin(pinName) : ShaderValuePinPtr();
	if (!pin)
	{
		error("Node has no input pin " + pinName);
		return ShaderValue();
	}

	return input(pin);
}

ShaderValue MaterialCompileContext::coerce(const ShaderValue& value, eShaderValueType targetType)
{
	// An invalid value has already been reported
	if (!value.isValid())
		return ShaderValue();

	if (value.type == targetType || targetType == eShaderValueType::wildcard)
		return value;

	if (value.type == eShaderValueType::float1 && ShaderValueTypeUtils::isFloatVector(targetType))
	{
		return {m_writer.broadcast(value.expr, targetType), targetType};
	}

	error("Expected " + ShaderValueTypeUtils::toString(targetType) + " but got "
		  + ShaderValueTypeUtils::toString(value.type));
	return ShaderValue();
}

// -- Emission -----
ShaderValue MaterialCompileContext::emitTemp(eShaderValueType type, const std::string& expr)
{
	StageBuilder& builder= stage();
	const std::string name= "t" + std::to_string(builder.nextTempIndex++);

	builder.statements.push_back(m_writer.declareTemp(type, name, expr));

	return {name, type};
}

ShaderValue MaterialCompileContext::uniform(const std::string& name, eShaderValueType type, eUniformSemantic semantic)
{
	auto it= m_uniforms.find(name);
	if (it != m_uniforms.end())
	{
		if (it->second.type != type)
		{
			error("Uniform " + name + " declared with two types");
			return ShaderValue();
		}
	}
	else
	{
		m_uniforms.insert({name, UniformEntry{type, semantic}});
		m_uniformOrder.push_back(name);
	}

	StageBuilder& builder= stage();
	if (builder.declaredUniforms.find(name) == builder.declaredUniforms.end())
	{
		builder.uniformDeclarations.push_back(m_writer.declareUniform(type, name));
		builder.declaredUniforms.insert(name);
	}

	return {name, type};
}

ShaderValue MaterialCompileContext::vertexAttribute(eVertexSemantic semantic)
{
	const eMaterialVertexPreset preset= getVertexPreset();
	const MaterialVertexAttribute* attribute= MaterialDomainUtils::findPresetAttribute(preset, semantic);
	if (!attribute)
	{
		error("Vertex preset " + MaterialDomainUtils::presetToString(preset) + " has no "
			  + VertexConstantUtils::vertexSemanticToString(semantic) + " attribute");
		return ShaderValue();
	}

	if (m_stage == eShaderStage::vertex)
	{
		ensureVertexInputDeclared(eShaderStage::vertex, *attribute);
		return {attribute->name, attribute->valueType};
	}

	// The fragment stage reads the attribute through a varying the vertex stage assigns
	auto it= m_varyings.find(semantic);
	if (it == m_varyings.end())
	{
		const std::string varyingName= makeVaryingName(attribute->name);
		StageBuilder& vertexBuilder= m_stages[(int)eShaderStage::vertex];
		StageBuilder& fragmentBuilder= m_stages[(int)eShaderStage::fragment];

		vertexBuilder.varyingDeclarations.push_back(
			m_writer.declareVarying(eShaderStage::vertex, attribute->valueType, varyingName));
		fragmentBuilder.varyingDeclarations.push_back(
			m_writer.declareVarying(eShaderStage::fragment, attribute->valueType, varyingName));
		vertexBuilder.varyingAssignments.push_back(m_writer.assignVarying(varyingName, attribute->name));
		ensureVertexInputDeclared(eShaderStage::vertex, *attribute);

		it= m_varyings.insert({semantic, varyingName}).first;
	}

	return {it->second, attribute->valueType};
}

ShaderValue MaterialCompileContext::fragmentCoord()
{
	if (m_stage != eShaderStage::fragment)
	{
		error("Fragment position is not available in the vertex stage");
		return ShaderValue();
	}

	return {m_writer.fragmentCoord(), eShaderValueType::float2};
}

ShaderValue MaterialCompileContext::callFunction(eShaderValueType returnType, const std::string& name,
												 const std::vector<ShaderFunctionParam>& params,
												 const std::string& body, const std::vector<ShaderValue>& args)
{
	StageBuilder& builder= stage();
	if (builder.declaredFunctions.find(name) == builder.declaredFunctions.end())
	{
		builder.functionDeclarations.push_back(m_writer.declareFunction(returnType, name, params, body));
		builder.declaredFunctions.insert(name);
	}

	std::vector<std::string> argExprs;
	argExprs.reserve(args.size());
	for (const ShaderValue& arg : args)
	{
		argExprs.push_back(arg.expr);
	}

	return {m_writer.callFunction(name, argExprs), returnType};
}

// -- Outputs -----
void MaterialCompileContext::setOutput(ShaderValuePinPtr pin, const ShaderValue& value)
{
	ShaderNodePtr node= getCurrentNode();
	if (!node)
	{
		error("Output set outside of a node compile", pin);
		return;
	}

	if (!pin)
	{
		error("Output pin is missing");
		return;
	}

	m_nodeOutputs[NodeStageKey(node->getId(), m_stage)][pin->getId()]= value;
	if (value.isValid())
	{
		m_resolvedPinTypes[pin->getId()]= value.type;
	}
}

void MaterialCompileContext::setOutput(const std::string& pinName, const ShaderValue& value)
{
	ShaderNodePtr node= getCurrentNode();
	ShaderValuePinPtr pin= node ? node->getShaderOutputPin(pinName) : ShaderValuePinPtr();
	if (!pin)
	{
		error("Node has no output pin " + pinName);
		return;
	}

	setOutput(pin, value);
}

// -- Diagnostics -----
void MaterialCompileContext::error(const std::string& message, NodePinPtr pin)
{
	ShaderNodePtr node= getCurrentNode();

	m_result.errors.push_back(
		NodeEvaluationError(eNodeEvaluationErrorCode::materialError, message, node.get(), pin.get()));
}

void MaterialCompileContext::setGlslOnly() { m_result.glslOnly= true; }

void MaterialCompileContext::addParameterDefault(const MaterialParameterDefault& parameterDefault)
{
	for (const MaterialParameterDefault& existing : m_result.parameterDefaults)
	{
		if (existing.name == parameterDefault.name)
			return;
	}

	m_result.parameterDefaults.push_back(parameterDefault);
}

// -- Driver -----
ShaderValue MaterialCompileContext::compileRoot(ShaderValuePinPtr outputPin, eShaderStage inStage)
{
	m_stage= inStage;

	if (!outputPin)
	{
		error("Output pin is missing");
		return ShaderValue();
	}

	// The root is an input pin of the output node, so it resolves exactly like any other input
	return input(outputPin);
}

std::string MaterialCompileContext::buildStageSource(eShaderStage inStage, const std::vector<std::string>& mainPrologue,
													 const std::vector<std::string>& mainEpilogue) const
{
	const StageBuilder& builder= stage(inStage);
	std::vector<std::string> lines;

	lines.push_back(m_writer.header(inStage));

	// One blank line between non-empty declaration groups, like the hand-written shaders
	bool bFirstGroup= true;
	auto appendGroup= [&lines, &bFirstGroup](const std::vector<std::string>& group, bool bSeparateEntries)
	{
		if (group.empty())
			return;

		if (!bFirstGroup)
		{
			lines.push_back("");
		}
		bFirstGroup= false;

		if (bSeparateEntries)
		{
			for (size_t index= 0; index < group.size(); ++index)
			{
				if (index > 0)
				{
					lines.push_back("");
				}
				lines.push_back(group[index]);
			}
		}
		else
		{
			appendLines(lines, group);
		}
	};

	appendGroup(builder.attributeDeclarations, false);
	appendGroup(builder.uniformDeclarations, false);
	appendGroup(builder.varyingDeclarations, false);
	appendGroup(builder.functionDeclarations, true);
	appendGroup(builder.outputDeclarations, false);

	lines.push_back("");
	lines.push_back(m_writer.beginMain());
	appendLines(lines, mainPrologue);
	appendLines(lines, builder.statements);
	if (inStage == eShaderStage::vertex)
	{
		appendLines(lines, builder.varyingAssignments);
	}
	appendLines(lines, mainEpilogue);
	lines.push_back(m_writer.endMain());

	std::string source;
	for (const std::string& line : lines)
	{
		source+= line;
		source+= "\n";
	}

	return source;
}

void MaterialCompileContext::declareMatrixUniform(const std::string& name, eUniformSemantic semantic)
{
	// A matrix is never a graph value, so it registers with no value type and
	// only needs to reach the semantic map
	auto it= m_uniforms.find(name);
	if (it != m_uniforms.end())
	{
		if (it->second.type != eShaderValueType::INVALID)
		{
			error("Uniform " + name + " declared with two types");
			return;
		}
	}
	else
	{
		m_uniforms.insert({name, UniformEntry{eShaderValueType::INVALID, semantic}});
		m_uniformOrder.push_back(name);
	}

	StageBuilder& builder= stage();
	if (builder.declaredUniforms.find(name) == builder.declaredUniforms.end())
	{
		builder.uniformDeclarations.push_back(m_writer.declareMatrixUniform(name));
		builder.declaredUniforms.insert(name);
	}
}

void MaterialCompileContext::applyResolvedTypes() const
{
	if (!m_graph)
		return;

	// Every pin the compile did not reach falls back to its declared type
	for (const auto& entry : m_graph->getNodesMap())
	{
		NodePtr node= entry.second;
		for (NodePinPtr pin : node->getInputPins())
		{
			auto shaderPin= std::dynamic_pointer_cast<ShaderValuePin>(pin);
			if (shaderPin)
			{
				shaderPin->setResolvedType(eShaderValueType::INVALID);
			}
		}
		for (NodePinPtr pin : node->getOutputPins())
		{
			auto shaderPin= std::dynamic_pointer_cast<ShaderValuePin>(pin);
			if (shaderPin)
			{
				shaderPin->setResolvedType(eShaderValueType::INVALID);
			}
		}
	}

	for (const auto& entry : m_resolvedPinTypes)
	{
		auto shaderPin= std::dynamic_pointer_cast<ShaderValuePin>(m_graph->getPinById(entry.first));
		if (shaderPin)
		{
			shaderPin->setResolvedType(entry.second);
		}
	}
}

eUniformSemantic MaterialCompileContext::getUniformSemantic(const std::string& name) const
{
	auto it= m_uniforms.find(name);

	return (it != m_uniforms.end()) ? it->second.semantic : eUniformSemantic::INVALID;
}

// -- Private -----
bool MaterialCompileContext::compileNode(ShaderNodePtr node)
{
	const NodeStageKey key(node->getId(), m_stage);

	// A node still on the stack already has its (empty) outputs entry, so the
	// cycle check has to run before the memo check
	if (m_visiting.find(key) != m_visiting.end())
	{
		m_result.errors.push_back(
			NodeEvaluationError(eNodeEvaluationErrorCode::materialError, "Cycle through node", node.get()));
		return false;
	}

	if (m_nodeOutputs.find(key) != m_nodeOutputs.end())
		return true;

	m_visiting.insert(key);
	m_nodeStack.push_back(node);
	// The entry exists before the node runs so setOutput has a target, and a
	// failed node leaves whatever it managed to produce
	m_nodeOutputs.insert({key, {}});

	const bool bSuccess= node->compileNode(*this);

	m_nodeStack.pop_back();
	m_visiting.erase(key);

	return bSuccess;
}

ShaderValue MaterialCompileContext::lookupOutput(ShaderValuePinPtr pin)
{
	NodePtr ownerNode= pin ? pin->getOwnerNode() : NodePtr();
	if (!ownerNode)
		return ShaderValue();

	auto nodeIt= m_nodeOutputs.find(NodeStageKey(ownerNode->getId(), m_stage));
	if (nodeIt == m_nodeOutputs.end())
		return ShaderValue();

	auto pinIt= nodeIt->second.find(pin->getId());

	return (pinIt != nodeIt->second.end()) ? pinIt->second : ShaderValue();
}

void MaterialCompileContext::ensureVertexInputDeclared(eShaderStage inStage, const MaterialVertexAttribute& attribute)
{
	StageBuilder& builder= m_stages[(int)inStage];
	if (builder.declaredAttributes.find(attribute.semantic) != builder.declaredAttributes.end())
		return;

	// The layout location is the attribute's index in the preset
	const std::vector<MaterialVertexAttribute>& attributes= MaterialDomainUtils::getPresetAttributes(getVertexPreset());
	int location= 0;
	for (size_t index= 0; index < attributes.size(); ++index)
	{
		if (attributes[index].semantic == attribute.semantic)
		{
			location= (int)index;
			break;
		}
	}

	builder.attributeDeclarations.push_back(m_writer.declareVertexInput(location, attribute.valueType, attribute.name));
	builder.declaredAttributes.insert(attribute.semantic);
}

// -- MaterialCompiler -----
MaterialCompileResult MaterialCompiler::compile(MaterialNodeGraphPtr graph, const IShaderWriter& writer)
{
	MaterialCompileResult result;

	if (!graph)
	{
		result.errors.push_back(
			NodeEvaluationError(eNodeEvaluationErrorCode::materialError, "No material graph to compile"));
		return result;
	}

	const eMaterialDomain domain= graph->getDomain();
	const eMaterialVertexPreset preset= graph->getVertexPreset();
	const std::vector<MaterialVertexAttribute>& attributes= MaterialDomainUtils::getPresetAttributes(preset);

	result.config->domain= MaterialDomainUtils::domainToString(domain);
	result.config->vertexPreset= MaterialDomainUtils::presetToString(preset);
	for (const MaterialVertexAttribute& attribute : attributes)
	{
		auto attributeConfig= std::make_shared<MikanVertexAttributeConfig>();
		attributeConfig->name= attribute.name;
		attributeConfig->dataType= attribute.dataType;
		attributeConfig->semantic= attribute.semantic;
		result.config->vertexAttributes.push_back(attributeConfig);
	}

	MaterialOutputNodePtr outputNode= graph->getOutputNode();
	if (!outputNode)
	{
		result.errors.push_back(
			NodeEvaluationError(eNodeEvaluationErrorCode::materialError, "Material graph has no output node"));
		return result;
	}

	MaterialCompileContext context(graph, writer, result);

	// Fragment stage: the color root
	std::vector<std::string> fragmentEpilogue;
	{
		ShaderValue color= context.compileRoot(outputNode->getColorPin(), eShaderStage::fragment);
		color= context.coerce(color, eShaderValueType::float4);
		if (color.isValid())
		{
			fragmentEpilogue.push_back(writer.assignFragmentColor(color.expr));
		}
	}

	// Vertex stage: the projection, plus the position offset root in the shape domain
	std::vector<std::string> vertexEpilogue;
	{
		const MaterialVertexAttribute* positionAttribute=
			MaterialDomainUtils::findPresetAttribute(preset, eVertexSemantic::position);
		if (positionAttribute)
		{
			if (domain == eMaterialDomain::shape)
			{
				// compileRoot selects the vertex stage, which the matrix uniform declaration relies on.
				// An unconnected offset pin compiles to its default literal, which is then ignored.
				ShaderValuePinPtr offsetPin= outputNode->getPositionOffsetPin();
				ShaderValue offset= context.compileRoot(offsetPin, eShaderStage::vertex);
				context.declareMatrixUniform(k_mvpUniformName, eUniformSemantic::modelViewProjectionMatrix);

				std::string positionExpr= positionAttribute->name;
				if (offsetPin && offsetPin->getConnectedSourcePin())
				{
					offset= context.coerce(offset, eShaderValueType::float3);
					if (offset.isValid())
					{
						positionExpr= writer.binaryOp(eShaderBinaryOp::add, positionAttribute->name, offset.expr);
					}
				}

				vertexEpilogue.push_back(writer.assignProjectedPosition(k_mvpUniformName, positionExpr));
			}
			else
			{
				vertexEpilogue.push_back(writer.assignClipPosition2D(positionAttribute->name));
			}
		}
		else
		{
			context.error("Vertex preset " + MaterialDomainUtils::presetToString(preset)
						  + " has no position attribute");
		}
	}

	result.vertexSource= context.buildStageSource(eShaderStage::vertex, {}, vertexEpilogue);
	result.fragmentSource= context.buildStageSource(eShaderStage::fragment, {}, fragmentEpilogue);

	for (const std::string& uniformName : context.getUniformNames())
	{
		result.config->uniformSemanticMap[uniformName]= getUniformSemanticName(context.getUniformSemantic(uniformName));
	}

	// Parameter defaults travel into the .mat so a consumer that never sets the uniform still binds something
	for (const MaterialParameterDefault& parameterDefault : result.parameterDefaults)
	{
		if (parameterDefault.type == eShaderValueType::texture2D)
		{
			if (!parameterDefault.textureAssetPath.empty())
			{
				result.config->uniformTextureDefaults[parameterDefault.name]= parameterDefault.textureAssetPath;
			}
		}
		else
		{
			const int componentCount= ShaderValueTypeUtils::getComponentCount(parameterDefault.type);
			if (componentCount > 0)
			{
				result.config->uniformFloatDefaults[parameterDefault.name]=
					std::vector<float>(parameterDefault.value.begin(), parameterDefault.value.begin() + componentCount);
			}
		}
	}

	context.applyResolvedTypes();

	return result;
}

bool MaterialCompiler::writeOutputs(MaterialCompileResult& result, const std::filesystem::path& graphPath,
									std::string& outError)
{
	if (result.hasErrors())
	{
		outError= "Material has compile errors";
		return false;
	}

	const std::string stem= graphPath.stem().string();
	if (stem.empty())
	{
		outError= "Graph path " + graphPath.string() + " has no file name";
		return false;
	}

	const std::filesystem::path vertexPath= getVertexShaderPathForGraph(graphPath);
	const std::filesystem::path fragmentPath= getFragmentShaderPathForGraph(graphPath);
	const std::filesystem::path materialPath= getMaterialPathForGraph(graphPath);

	if (!writeTextFile(vertexPath, result.vertexSource, outError)
		|| !writeTextFile(fragmentPath, result.fragmentSource, outError))
	{
		MIKAN_LOG_ERROR("MaterialCompiler::writeOutputs") << outError;
		return false;
	}

	// The shader cache resolves the shader paths against the .mat folder, so they stay relative
	result.config->materialName= stem;
	result.config->vertexShaderPath= vertexPath.filename();
	result.config->fragmentShaderPath= fragmentPath.filename();
	result.config->sourceGraphPath= graphPath.filename();

	try
	{
		result.config->save(materialPath);
	}
	catch (const std::exception& e)
	{
		outError= "Failed to write " + materialPath.string() + ": " + e.what();
		MIKAN_LOG_ERROR("MaterialCompiler::writeOutputs") << outError;
		return false;
	}

	std::error_code existsError;
	if (!std::filesystem::exists(materialPath, existsError))
	{
		outError= "Failed to write " + materialPath.string();
		MIKAN_LOG_ERROR("MaterialCompiler::writeOutputs") << outError;
		return false;
	}

	return true;
}

std::filesystem::path MaterialCompiler::getMaterialPathForGraph(const std::filesystem::path& graphPath)
{
	return graphPath.parent_path() / (graphPath.stem().string() + ".mat");
}

std::filesystem::path MaterialCompiler::getVertexShaderPathForGraph(const std::filesystem::path& graphPath)
{
	return graphPath.parent_path() / (graphPath.stem().string() + ".vert");
}

std::filesystem::path MaterialCompiler::getFragmentShaderPathForGraph(const std::filesystem::path& graphPath)
{
	return graphPath.parent_path() / (graphPath.stem().string() + ".frag");
}
