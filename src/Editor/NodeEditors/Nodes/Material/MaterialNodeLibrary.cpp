#include "MaterialNodeLibrary.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/ShaderMathOpTable.h"

#include "ShaderAppendNode.h"
#include "ShaderConstantNode.h"
#include "ShaderCustomExpressionNode.h"
#include "ShaderMathNode.h"
#include "ShaderParameterNode.h"
#include "ShaderSemanticInputNode.h"
#include "ShaderSwizzleNode.h"
#include "ShaderTextureParameterNode.h"
#include "ShaderTextureSampleNode.h"
#include "ShaderVertexInputNode.h"

namespace MaterialNodeLibrary
{
void registerNodeFactories(NodeGraph& graph)
{
	// Constants
	graph.addNodeFactory<ShaderConstantNodeFactory>(eShaderValueType::float1, false);
	graph.addNodeFactory<ShaderConstantNodeFactory>(eShaderValueType::float2, false);
	graph.addNodeFactory<ShaderConstantNodeFactory>(eShaderValueType::float3, false);
	graph.addNodeFactory<ShaderConstantNodeFactory>(eShaderValueType::float4, false);
	graph.addNodeFactory<ShaderConstantNodeFactory>(eShaderValueType::float4, true);

	// Parameters
	graph.addNodeFactory<ShaderParameterNodeFactory>(eShaderParameterVariant::float1);
	graph.addNodeFactory<ShaderParameterNodeFactory>(eShaderParameterVariant::float2);
	graph.addNodeFactory<ShaderParameterNodeFactory>(eShaderParameterVariant::float3);
	graph.addNodeFactory<ShaderParameterNodeFactory>(eShaderParameterVariant::float4);
	graph.addNodeFactory<ShaderParameterNodeFactory>(eShaderParameterVariant::color);
	graph.addNodeFactory<ShaderParameterNodeFactory>(eShaderParameterVariant::time);
	graph.addNodeFactory<ShaderTextureParameterNodeFactory>();

	// Texture
	graph.addNodeFactory<ShaderTextureSampleNodeFactory>();

	// Vertex inputs
	graph.addNodeFactory<ShaderVertexInputNodeFactory>(eVertexSemantic::position);
	graph.addNodeFactory<ShaderVertexInputNodeFactory>(eVertexSemantic::normal);
	graph.addNodeFactory<ShaderVertexInputNodeFactory>(eVertexSemantic::texCoord);
	graph.addNodeFactory<ShaderVertexInputNodeFactory>(eVertexSemantic::color);

	// Semantic inputs
	graph.addNodeFactory<ShaderSemanticInputNodeFactory>(eUniformSemantic::screenSize);
	graph.addNodeFactory<ShaderSemanticInputNodeFactory>(eUniformSemantic::screenPosition);
	graph.addNodeFactory<ShaderSemanticInputNodeFactory>(eUniformSemantic::cameraPosition);
	graph.addNodeFactory<ShaderSemanticInputNodeFactory>(eUniformSemantic::zNear);
	graph.addNodeFactory<ShaderSemanticInputNodeFactory>(eUniformSemantic::zFar);

	// Math: one create menu entry per table row
	for (eShaderMathOp op : ShaderMathOpTable::getAllOps())
	{
		graph.addNodeFactory<ShaderMathNodeFactory>(op);
	}

	// Vector
	graph.addNodeFactory<ShaderSwizzleNodeFactory>();
	graph.addNodeFactory<ShaderAppendNodeFactory>();

	// Custom
	graph.addNodeFactory<ShaderCustomExpressionNodeFactory>();
}
} // namespace MaterialNodeLibrary
