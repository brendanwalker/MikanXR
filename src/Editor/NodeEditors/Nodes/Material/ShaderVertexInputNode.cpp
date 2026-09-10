#include "ShaderVertexInputNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "NodeEditorState.h"
#include "Graphs/MaterialNodeGraph.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"
#include "MaterialCompiler/MaterialDomain.h"

namespace
{
// The type a semantic carries when the graph's preset lacks it, so the pin
// still declares something sensible until a preset that has it is chosen
eShaderValueType getFallbackVertexInputType(eVertexSemantic semantic)
{
	switch (semantic)
	{
	case eVertexSemantic::texCoord:
		return eShaderValueType::float2;
	case eVertexSemantic::color:
		return eShaderValueType::float4;
	default:
		return eShaderValueType::float3;
	}
}
} // namespace

// -- ShaderVertexInputNodeConfig -----
configuru::Config ShaderVertexInputNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["semantic"]= semantic;

	return pt;
}

void ShaderVertexInputNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	semantic=
		pt.get_or<std::string>("semantic", VertexConstantUtils::vertexSemanticToString(eVertexSemantic::position));
}

// -- ShaderVertexInputNode -----
bool ShaderVertexInputNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderVertexInputNodeConfig>(nodeConfig);

		const eVertexSemantic semantic= VertexConstantUtils::vertexSemanticFromString(config->semantic);
		m_semantic= (semantic != eVertexSemantic::INVALID) ? semantic : eVertexSemantic::position;

		return true;
	}

	return false;
}

void ShaderVertexInputNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderVertexInputNodeConfig>(nodeConfig);

	config->semantic= VertexConstantUtils::vertexSemanticToString(m_semantic);

	Node::saveToConfig(nodeConfig);
}

bool ShaderVertexInputNode::compileNode(MaterialCompileContext& context)
{
	const ShaderValue value= context.vertexAttribute(m_semantic);
	if (!value.isValid())
		return false;

	// The preset decides the width (a vec2 position on the compositor quad, vec3
	// on shapes), so the pin follows what the compile actually produced
	ShaderValuePinPtr valuePin= getShaderOutputPin(k_valuePinName);
	if (valuePin)
	{
		valuePin->setDeclaredType(value.type);
	}

	context.setOutput(k_valuePinName, value);

	return true;
}

std::string ShaderVertexInputNode::editorGetTitle() const
{
	switch (m_semantic)
	{
	case eVertexSemantic::normal:
		return locText("nodes.vertexNormalTitle");
	case eVertexSemantic::texCoord:
		return locText("nodes.texCoordTitle");
	case eVertexSemantic::color:
		return locText("nodes.vertexColorTitle");
	default:
		return locText("nodes.vertexPositionTitle");
	}
}

const char* ShaderVertexInputNode::editorGetHeaderIcon() const { return ICON_FK_CUBE; }

// -- ShaderVertexInputNodeFactory -----
ShaderVertexInputNodeFactory::ShaderVertexInputNodeFactory(eVertexSemantic semantic)
	: m_semantic(semantic)
{
}

std::string ShaderVertexInputNodeFactory::getFactoryKey() const
{
	return ShaderVertexInputNode::k_nodeClassName + ":" + VertexConstantUtils::vertexSemanticToString(m_semantic);
}

std::string ShaderVertexInputNodeFactory::editorGetCategory() const { return locText("nodes.categoryInputs"); }

NodePtr ShaderVertexInputNodeFactory::allocateNode() const
{
	auto node= std::make_shared<ShaderVertexInputNode>();
	node->setSemantic(m_semantic);

	return node;
}

NodePtr ShaderVertexInputNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderVertexInputNode>(NodeFactory::createNode(editorState));

	// Declare the type the graph's preset gives the attribute, when it has one
	eShaderValueType valueType= getFallbackVertexInputType(m_semantic);
	auto materialGraph= std::dynamic_pointer_cast<MaterialNodeGraph>(editorState.nodeGraph);
	if (materialGraph)
	{
		const MaterialVertexAttribute* attribute=
			MaterialDomainUtils::findPresetAttribute(materialGraph->getVertexPreset(), m_semantic);
		if (attribute)
		{
			valueType= attribute->valueType;
		}
	}

	ShaderValuePinPtr outputPin= node->addShaderOutputPin(ShaderVertexInputNode::k_valuePinName, valueType);

	autoConnectOutputPin(editorState, outputPin);

	return node;
}
