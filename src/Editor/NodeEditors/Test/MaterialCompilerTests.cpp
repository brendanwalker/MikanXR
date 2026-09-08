#include "MaterialCompilerTests.h"
#include "unit_test.h"

#include "NodeEditorState.h"
#include "Graphs/MaterialNodeGraph.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/GlslShaderWriter.h"
#include "MaterialCompiler/MaterialCompiler.h"
#include "Nodes/Material/MaterialOutputNode.h"
#include "Nodes/Material/ShaderConstantNode.h"
#include "Nodes/Material/ShaderNode.h"
#include "Nodes/Material/ShaderParameterNode.h"
#include "Nodes/Material/ShaderTextureParameterNode.h"
#include "Pins/ShaderValuePin.h"
#include "MikanShaderConfig.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// The compiler never touches the owner window or a GL resource, so every graph
// here is built with no window at all.

namespace
{
MaterialNodeGraphPtr makeMaterialGraph(eMaterialDomain domain)
{
	MaterialNodeGraphFactory factory;
	NodeGraphPtr graph= factory.initialCreateMaterialGraph(nullptr, domain);

	return std::static_pointer_cast<MaterialNodeGraph>(graph);
}

ShaderNodePtr makeNode(MaterialNodeGraphPtr graph, const std::string& factoryKey)
{
	NodeFactoryPtr factory= graph->getNodeFactory(factoryKey);
	if (!factory)
	{
		fprintf(stdout, "    FAILED: no node factory registered for %s\n", factoryKey.c_str());
		return ShaderNodePtr();
	}

	NodeEditorState editorState;
	editorState.nodeGraph= graph;

	return std::dynamic_pointer_cast<ShaderNode>(graph->createNode(factory, editorState));
}

bool link(MaterialNodeGraphPtr graph, ShaderNodePtr fromNode, const std::string& outputPin, ShaderNodePtr toNode,
		  const std::string& inputPin)
{
	ShaderValuePinPtr from= fromNode ? fromNode->getShaderOutputPin(outputPin) : ShaderValuePinPtr();
	ShaderValuePinPtr to= toNode ? toNode->getShaderInputPin(inputPin) : ShaderValuePinPtr();
	if (!from || !to)
	{
		fprintf(stdout, "    FAILED: missing pin %s or %s\n", outputPin.c_str(), inputPin.c_str());
		return false;
	}

	return graph->createLink(from->getId(), to->getId()) != nullptr;
}

bool contains(const std::string& text, const std::string& needle) { return text.find(needle) != std::string::npos; }

bool expectContains(const std::string& text, const std::string& needle, const char* what)
{
	if (!contains(text, needle))
	{
		fprintf(stdout, "    FAILED: %s is missing \"%s\"\n%s\n", what, needle.c_str(), text.c_str());
		return false;
	}

	return true;
}

bool expectNoErrors(const MaterialCompileResult& result)
{
	for (const NodeEvaluationError& error : result.errors)
	{
		fprintf(stdout, "    FAILED: unexpected compile error (node %d): %s\n", error.errorNodeId,
				error.errorMessage.c_str());
	}

	return !result.hasErrors();
}

bool expectErrorMentioning(const MaterialCompileResult& result, const std::string& fragment, t_node_id nodeId= -1)
{
	for (const NodeEvaluationError& error : result.errors)
	{
		if (contains(error.errorMessage, fragment) && (nodeId == -1 || error.errorNodeId == nodeId))
			return true;
	}

	fprintf(stdout, "    FAILED: no compile error mentioning \"%s\"", fragment.c_str());
	if (nodeId != -1)
	{
		fprintf(stdout, " on node %d", nodeId);
	}
	fprintf(stdout, " (%zu errors)\n", result.errors.size());
	for (const NodeEvaluationError& error : result.errors)
	{
		fprintf(stdout, "      node %d: %s\n", error.errorNodeId, error.errorMessage.c_str());
	}

	return false;
}

std::string readTextFile(const std::filesystem::path& path)
{
	std::ifstream file(path, std::ios::binary);
	std::stringstream buffer;
	buffer << file.rdbuf();

	// Line endings are compared loosely so a checkout with autocrlf still passes
	std::string text= buffer.str();
	text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());

	return text;
}

// The passthrough every compositor layer material starts from: sample a texture parameter into the color
MaterialNodeGraphPtr makeCompositorPassthroughGraph(ShaderNodePtr& outSampleNode)
{
	MaterialNodeGraphPtr graph= makeMaterialGraph(eMaterialDomain::compositor);

	auto textureNode= std::dynamic_pointer_cast<ShaderTextureParameterNode>(
		makeNode(graph, ShaderTextureParameterNode::k_nodeClassName));
	if (textureNode)
	{
		textureNode->setParameterName("rgbaTexture");
	}
	outSampleNode= makeNode(graph, "ShaderTextureSampleNode");

	link(graph, textureNode, "texture", outSampleNode, "texture");
	link(graph, outSampleNode, "rgba", graph->getOutputNode(), MaterialOutputNode::k_colorPinName);

	return graph;
}
} // namespace

// -- tests -----

bool material_compiler_test_compositor_passthrough()
{
	UNIT_TEST_BEGIN("a compositor texture passthrough compiles to the layer quad interface")

	ShaderNodePtr sampleNode;
	MaterialNodeGraphPtr graph= makeCompositorPassthroughGraph(sampleNode);

	GlslShaderWriter writer;
	MaterialCompileResult result= graph->compile(writer);
	success&= expectNoErrors(result);

	// Vertex stage: the P2T attributes in location order, the varying feeding the sample, the clip position
	success&= expectContains(result.vertexSource, "#version 330 core", "vertex source");
	success&= expectContains(result.vertexSource, "layout (location = 0) in vec2 aPos;", "vertex source");
	success&= expectContains(result.vertexSource, "layout (location = 1) in vec2 aTexCoords;", "vertex source");
	success&= expectContains(result.vertexSource, "out vec2 vTexCoords;", "vertex source");
	success&= expectContains(result.vertexSource, "\tvTexCoords = aTexCoords;", "vertex source");
	success&= expectContains(result.vertexSource, "\tgl_Position = vec4(aPos, 0.0, 1.0);", "vertex source");

	// Fragment stage: the sampler uniform, the varying, the sample, the color write
	success&= expectContains(result.fragmentSource, "uniform sampler2D rgbaTexture;", "fragment source");
	success&= expectContains(result.fragmentSource, "in vec2 vTexCoords;", "fragment source");
	success&= expectContains(result.fragmentSource, "out vec4 FragColor;", "fragment source");
	success&= expectContains(result.fragmentSource, "texture(rgbaTexture, vTexCoords)", "fragment source");
	success&= expectContains(result.fragmentSource, "\tFragColor = ", "fragment source");

	// The .mat config
	success&= result.config->domain == "compositor";
	success&= result.config->vertexPreset == "P2T";
	success&= result.config->vertexAttributes.size() == 2;
	success&= result.config->uniformSemanticMap.size() == 1;
	success&= result.config->uniformSemanticMap["rgbaTexture"] == "textureParam";

	// A texture parameter default reaches the preview
	success&= result.parameterDefaults.size() == 1 && result.parameterDefaults[0].name == "rgbaTexture";

	// The compile wrote resolved types back onto the pins it walked
	ShaderValuePinPtr rgbaPin= sampleNode ? sampleNode->getShaderOutputPin("rgba") : ShaderValuePinPtr();
	success&= rgbaPin && rgbaPin->getResolvedType() == eShaderValueType::float4;

	UNIT_TEST_COMPLETE()
}

bool material_compiler_test_shape_vertex_stage()
{
	UNIT_TEST_BEGIN("a shape material with a position offset splits work across both stages")

	MaterialNodeGraphPtr graph= makeMaterialGraph(eMaterialDomain::shape);
	graph->setVertexPreset(eMaterialVertexPreset::PNT);

	// color= append(normal, 1.0), positionOffset= normal * extrude
	ShaderNodePtr normalNode= makeNode(graph, "ShaderVertexInputNode:normal");
	ShaderNodePtr oneNode= makeNode(graph, "ShaderConstantNode:float");
	if (auto constantNode= std::dynamic_pointer_cast<ShaderConstantNode>(oneNode))
	{
		constantNode->setValue({1.f, 0.f, 0.f, 0.f});
	}
	ShaderNodePtr appendNode= makeNode(graph, "ShaderAppendNode");
	auto extrudeNode= std::dynamic_pointer_cast<ShaderParameterNode>(makeNode(graph, "ShaderParameterNode:float"));
	if (extrudeNode)
	{
		extrudeNode->setParameterName("extrude");
		extrudeNode->setDefaultValue({0.25f, 0.f, 0.f, 0.f});
	}
	ShaderNodePtr multiplyNode= makeNode(graph, "ShaderMathNode:multiply");

	MaterialOutputNodePtr outputNode= graph->getOutputNode();
	success&= outputNode && outputNode->getPositionOffsetPin() != nullptr;

	success&= link(graph, normalNode, "value", appendNode, "a");
	success&= link(graph, oneNode, "value", appendNode, "b");
	success&= link(graph, appendNode, "result", outputNode, MaterialOutputNode::k_colorPinName);
	success&= link(graph, normalNode, "value", multiplyNode, "a");
	success&= link(graph, extrudeNode, "value", multiplyNode, "b");
	success&= link(graph, multiplyNode, "result", outputNode, MaterialOutputNode::k_positionOffsetPinName);

	GlslShaderWriter writer;
	MaterialCompileResult result= graph->compile(writer);
	success&= expectNoErrors(result);

	// Vertex stage: PNT attributes, the projection matrix, the offset temp, the varying for the fragment normal
	success&= expectContains(result.vertexSource, "layout (location = 1) in vec3 aNormal;", "vertex source");
	success&= expectContains(result.vertexSource, "layout (location = 2) in vec2 aTexCoords;", "vertex source");
	success&= expectContains(result.vertexSource, "uniform mat4 mvpMatrix;", "vertex source");
	success&= expectContains(result.vertexSource, "uniform float extrude;", "vertex source");
	success&= expectContains(result.vertexSource, "out vec3 vNormal;", "vertex source");
	success&= expectContains(result.vertexSource, "\tvNormal = aNormal;", "vertex source");
	success&= expectContains(result.vertexSource, "(aNormal * vec3(extrude))", "vertex source");
	success&=
		expectContains(result.vertexSource, "\tgl_Position = mvpMatrix * vec4((aPos + t0), 1.0);", "vertex source");

	// Fragment stage reads the normal through the varying, and never sees the extrude uniform
	success&= expectContains(result.fragmentSource, "in vec3 vNormal;", "fragment source");
	success&= expectContains(result.fragmentSource, "vec4(vNormal, 1.0)", "fragment source");
	success&= !contains(result.fragmentSource, "extrude");
	success&= !contains(result.fragmentSource, "mvpMatrix");

	success&= result.config->domain == "shape";
	success&= result.config->vertexPreset == "PNT";
	success&= result.config->vertexAttributes.size() == 3;
	success&= result.config->uniformSemanticMap["mvpMatrix"] == "modelViewProjectionMatrix";
	success&= result.config->uniformSemanticMap["extrude"] == "floatParam";

	UNIT_TEST_COMPLETE()
}

bool material_compiler_test_scalar_broadcast()
{
	UNIT_TEST_BEGIN("a float1 operand broadcasts into a wider vector operand")

	MaterialNodeGraphPtr graph= makeMaterialGraph(eMaterialDomain::compositor);

	ShaderNodePtr colorNode= makeNode(graph, "ShaderConstantNode:float4");
	if (auto constantNode= std::dynamic_pointer_cast<ShaderConstantNode>(colorNode))
	{
		constantNode->setValue({1.f, 0.5f, 0.25f, 1.f});
	}
	ShaderNodePtr scaleNode= makeNode(graph, "ShaderConstantNode:float");
	if (auto constantNode= std::dynamic_pointer_cast<ShaderConstantNode>(scaleNode))
	{
		constantNode->setValue({0.5f, 0.f, 0.f, 0.f});
	}
	ShaderNodePtr multiplyNode= makeNode(graph, "ShaderMathNode:multiply");

	success&= link(graph, colorNode, "value", multiplyNode, "a");
	success&= link(graph, scaleNode, "value", multiplyNode, "b");
	success&= link(graph, multiplyNode, "result", graph->getOutputNode(), MaterialOutputNode::k_colorPinName);

	GlslShaderWriter writer;
	MaterialCompileResult result= graph->compile(writer);
	success&= expectNoErrors(result);
	success&= expectContains(result.fragmentSource, "(vec4(1.0, 0.5, 0.25, 1.0) * vec4(0.5))", "fragment source");

	// An unconnected wildcard operand is a scalar literal of the pin default (Multiply's b defaults to 1)
	MaterialNodeGraphPtr defaultGraph= makeMaterialGraph(eMaterialDomain::compositor);
	ShaderNodePtr color2Node= makeNode(defaultGraph, "ShaderConstantNode:float4");
	ShaderNodePtr multiply2Node= makeNode(defaultGraph, "ShaderMathNode:multiply");
	success&= link(defaultGraph, color2Node, "value", multiply2Node, "a");
	success&=
		link(defaultGraph, multiply2Node, "result", defaultGraph->getOutputNode(), MaterialOutputNode::k_colorPinName);
	MaterialCompileResult defaultResult= defaultGraph->compile(writer);
	success&= expectNoErrors(defaultResult);
	success&= expectContains(defaultResult.fragmentSource, "* vec4(1.0))", "fragment source");

	UNIT_TEST_COMPLETE()
}

bool material_compiler_test_type_mismatch_names_node()
{
	UNIT_TEST_BEGIN("a float2 times float3 is an error attributed to the math node")

	MaterialNodeGraphPtr graph= makeMaterialGraph(eMaterialDomain::compositor);

	ShaderNodePtr aNode= makeNode(graph, "ShaderConstantNode:float2");
	ShaderNodePtr bNode= makeNode(graph, "ShaderConstantNode:float3");
	ShaderNodePtr multiplyNode= makeNode(graph, "ShaderMathNode:multiply");

	success&= link(graph, aNode, "value", multiplyNode, "a");
	success&= link(graph, bNode, "value", multiplyNode, "b");
	success&= link(graph, multiplyNode, "result", graph->getOutputNode(), MaterialOutputNode::k_colorPinName);

	GlslShaderWriter writer;
	MaterialCompileResult result= graph->compile(writer);
	success&= result.hasErrors();
	success&= multiplyNode && expectErrorMentioning(result, "do not combine", multiplyNode->getId());

	// A texture cannot be left unconnected
	MaterialNodeGraphPtr textureGraph= makeMaterialGraph(eMaterialDomain::compositor);
	ShaderNodePtr sampleNode= makeNode(textureGraph, "ShaderTextureSampleNode");
	success&= link(textureGraph, sampleNode, "rgba", textureGraph->getOutputNode(), MaterialOutputNode::k_colorPinName);
	MaterialCompileResult textureResult= textureGraph->compile(writer);
	success&= sampleNode && expectErrorMentioning(textureResult, "needs a connection", sampleNode->getId());

	UNIT_TEST_COMPLETE()
}

bool material_compiler_test_cycle_is_an_error()
{
	UNIT_TEST_BEGIN("a link cycle is reported instead of recursing forever")

	MaterialNodeGraphPtr graph= makeMaterialGraph(eMaterialDomain::compositor);

	ShaderNodePtr firstAdd= makeNode(graph, "ShaderMathNode:add");
	ShaderNodePtr secondAdd= makeNode(graph, "ShaderMathNode:add");

	success&= link(graph, firstAdd, "result", secondAdd, "a");
	success&= link(graph, secondAdd, "result", firstAdd, "a");
	success&= link(graph, firstAdd, "result", graph->getOutputNode(), MaterialOutputNode::k_colorPinName);

	GlslShaderWriter writer;
	MaterialCompileResult result= graph->compile(writer);
	success&= expectErrorMentioning(result, "Cycle");

	UNIT_TEST_COMPLETE()
}

bool material_compiler_test_parameter_name_conflicts()
{
	UNIT_TEST_BEGIN("two parameters sharing a name with different types is an error")

	MaterialNodeGraphPtr graph= makeMaterialGraph(eMaterialDomain::compositor);

	auto floatParam= std::dynamic_pointer_cast<ShaderParameterNode>(makeNode(graph, "ShaderParameterNode:float"));
	auto float3Param= std::dynamic_pointer_cast<ShaderParameterNode>(makeNode(graph, "ShaderParameterNode:float3"));
	if (floatParam && float3Param)
	{
		floatParam->setParameterName("tint");
		float3Param->setParameterName("tint");
	}
	ShaderNodePtr appendNode= makeNode(graph, "ShaderAppendNode");

	success&= link(graph, float3Param, "value", appendNode, "a");
	success&= link(graph, floatParam, "value", appendNode, "b");
	success&= link(graph, appendNode, "result", graph->getOutputNode(), MaterialOutputNode::k_colorPinName);

	GlslShaderWriter writer;
	MaterialCompileResult result= graph->compile(writer);
	success&= expectErrorMentioning(result, "two types");

	// A name that is not an identifier is rejected before it reaches the shader
	MaterialNodeGraphPtr badNameGraph= makeMaterialGraph(eMaterialDomain::compositor);
	auto badParam= std::dynamic_pointer_cast<ShaderParameterNode>(makeNode(badNameGraph, "ShaderParameterNode:float4"));
	if (badParam)
	{
		badParam->setParameterName("2 tint");
	}
	success&= link(badNameGraph, badParam, "value", badNameGraph->getOutputNode(), MaterialOutputNode::k_colorPinName);
	MaterialCompileResult badNameResult= badNameGraph->compile(writer);
	success&= expectErrorMentioning(badNameResult, "identifier");

	UNIT_TEST_COMPLETE()
}

bool material_compiler_test_custom_expression_flags_glsl_only()
{
	UNIT_TEST_BEGIN("a custom expression compiles to a function and marks the material GLSL only")

	MaterialNodeGraphPtr graph= makeMaterialGraph(eMaterialDomain::compositor);

	ShaderNodePtr customNode= makeNode(graph, "ShaderCustomExpressionNode");
	success&= link(graph, customNode, "result", graph->getOutputNode(), MaterialOutputNode::k_colorPinName);

	GlslShaderWriter writer;
	MaterialCompileResult result= graph->compile(writer);
	success&= result.glslOnly;
	if (!result.hasErrors())
	{
		success&= expectContains(result.fragmentSource, "custom_", "fragment source");
	}

	UNIT_TEST_COMPLETE()
}

bool material_compiler_test_snapshot_round_trip()
{
	UNIT_TEST_BEGIN("a graph saved and reloaded through its snapshot compiles to the same source")

	NodeGraphFactory::registerFactory<MaterialNodeGraphFactory>();

	ShaderNodePtr sampleNode;
	MaterialNodeGraphPtr graph= makeCompositorPassthroughGraph(sampleNode);

	GlslShaderWriter writer;
	MaterialCompileResult before= graph->compile(writer);
	success&= expectNoErrors(before);

	const std::string snapshot= graph->saveToSnapshotString();
	NodeGraphPtr reloaded= NodeGraphFactory::loadNodeGraphFromSnapshotString(nullptr, snapshot);
	auto reloadedMaterialGraph= std::dynamic_pointer_cast<MaterialNodeGraph>(reloaded);
	success&= reloadedMaterialGraph != nullptr;

	if (reloadedMaterialGraph)
	{
		success&= reloadedMaterialGraph->getDomain() == eMaterialDomain::compositor;
		success&= reloadedMaterialGraph->getVertexPreset() == eMaterialVertexPreset::P2T;

		MaterialCompileResult after= reloadedMaterialGraph->compile(writer);
		success&= expectNoErrors(after);
		success&= after.vertexSource == before.vertexSource;
		success&= after.fragmentSource == before.fragmentSource;
	}

	UNIT_TEST_COMPLETE()
}

bool material_compiler_test_shipped_graphs_match_outputs()
{
	UNIT_TEST_BEGIN("every shipped material graph recompiles to its checked-in shaders and .mat")

	NodeGraphFactory::registerFactory<MaterialNodeGraphFactory>();

	const std::filesystem::path shadersRoot= std::filesystem::path("resources") / "shaders";
	if (!std::filesystem::exists(shadersRoot))
	{
		fprintf(stdout, "    FAILED: %s not found (run from the repo root)\n", shadersRoot.string().c_str());
		success= false;
	}

	int graphCount= 0;
	if (success)
	{
		for (const auto& entry : std::filesystem::recursive_directory_iterator(shadersRoot))
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".graph")
				continue;

			// Config loading resolves relative paths against the project and resource roots, so
			// hand the loader an absolute path
			const std::filesystem::path graphPath= std::filesystem::absolute(entry.path());
			graphCount++;

			NodeGraphPtr graph= NodeGraphFactory::loadNodeGraph(nullptr, graphPath);
			auto materialGraph= std::dynamic_pointer_cast<MaterialNodeGraph>(graph);
			if (!materialGraph)
			{
				fprintf(stdout, "    FAILED: %s is not a material graph\n", graphPath.string().c_str());
				success= false;
				continue;
			}

			GlslShaderWriter writer;
			MaterialCompileResult result= materialGraph->compile(writer);
			if (!expectNoErrors(result))
			{
				fprintf(stdout, "    FAILED: %s does not compile\n", graphPath.string().c_str());
				success= false;
				continue;
			}

			const std::string vertexOnDisk= readTextFile(MaterialCompiler::getVertexShaderPathForGraph(graphPath));
			const std::string fragmentOnDisk= readTextFile(MaterialCompiler::getFragmentShaderPathForGraph(graphPath));
			if (vertexOnDisk != result.vertexSource || fragmentOnDisk != result.fragmentSource)
			{
				fprintf(stdout, "    FAILED: %s is out of date with its graph (recompile and commit the outputs)\n",
						graphPath.string().c_str());
				success= false;
			}

			MikanShaderConfig materialOnDisk;
			if (!materialOnDisk.load(MaterialCompiler::getMaterialPathForGraph(graphPath)))
			{
				fprintf(stdout, "    FAILED: %s has no .mat beside it\n", graphPath.string().c_str());
				success= false;
				continue;
			}
			success&= materialOnDisk.domain == result.config->domain;
			success&= materialOnDisk.vertexPreset == result.config->vertexPreset;
			success&= materialOnDisk.uniformSemanticMap == result.config->uniformSemanticMap;
			success&= materialOnDisk.sourceGraphPath == graphPath.filename();
			success&= materialOnDisk.vertexAttributes.size() == result.config->vertexAttributes.size();
		}
	}

	fprintf(stdout, "      %d shipped material graph(s) checked\n", graphCount);
	success&= graphCount > 0;

	UNIT_TEST_COMPLETE()
}

// -- module -----

bool run_material_compiler_tests()
{
	UNIT_TEST_MODULE_BEGIN("material_compiler")
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_compositor_passthrough);
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_shape_vertex_stage);
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_scalar_broadcast);
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_type_mismatch_names_node);
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_cycle_is_an_error);
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_parameter_name_conflicts);
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_custom_expression_flags_glsl_only);
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_snapshot_round_trip);
	UNIT_TEST_MODULE_CALL_TEST(material_compiler_test_shipped_graphs_match_outputs);
	UNIT_TEST_MODULE_END()
}
