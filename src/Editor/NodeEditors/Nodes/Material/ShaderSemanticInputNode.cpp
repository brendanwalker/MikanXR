#include "ShaderSemanticInputNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

// -- ShaderSemanticInputNodeConfig -----
configuru::Config ShaderSemanticInputNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["semantic"]= semantic;

	return pt;
}

void ShaderSemanticInputNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	semantic= pt.get_or<std::string>("semantic", getUniformSemanticName(eUniformSemantic::screenSize));
}

// -- ShaderSemanticInputNode -----
bool ShaderSemanticInputNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderSemanticInputNodeConfig>(nodeConfig);

		const eUniformSemantic semantic= semanticFromName(config->semantic);
		m_semantic= (semantic != eUniformSemantic::INVALID) ? semantic : eUniformSemantic::screenSize;

		return true;
	}

	return false;
}

void ShaderSemanticInputNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderSemanticInputNodeConfig>(nodeConfig);

	config->semantic= getUniformSemanticName(m_semantic);

	Node::saveToConfig(nodeConfig);
}

eShaderValueType ShaderSemanticInputNode::getValueType() const
{
	return ShaderValueTypeUtils::fromUniformDataType(getUniformSemanticDataType(m_semantic));
}

bool ShaderSemanticInputNode::compileNode(MaterialCompileContext& context)
{
	const ShaderValue value= context.uniform(getUniformSemanticName(m_semantic), getValueType(), m_semantic);
	if (!value.isValid())
		return false;

	context.setOutput(k_valuePinName, value);

	return true;
}

std::string ShaderSemanticInputNode::editorGetTitle() const
{
	switch (m_semantic)
	{
	case eUniformSemantic::screenPosition:
		return locText("nodes.screenPositionTitle");
	case eUniformSemantic::cameraPosition:
		return locText("nodes.cameraPositionTitle");
	case eUniformSemantic::zNear:
		return locText("nodes.nearPlaneTitle");
	case eUniformSemantic::zFar:
		return locText("nodes.farPlaneTitle");
	default:
		return locText("nodes.screenSizeTitle");
	}
}

const char* ShaderSemanticInputNode::editorGetHeaderIcon() const
{
	return (m_semantic == eUniformSemantic::cameraPosition) ? ICON_FK_VIDEO_CAMERA : ICON_FK_DESKTOP;
}

eUniformSemantic ShaderSemanticInputNode::semanticFromName(const std::string& name)
{
	for (int index= 0; index < (int)eUniformSemantic::COUNT; ++index)
	{
		const eUniformSemantic semantic= (eUniformSemantic)index;
		if (getUniformSemanticName(semantic) == name)
			return semantic;
	}

	return eUniformSemantic::INVALID;
}

// -- ShaderSemanticInputNodeFactory -----
ShaderSemanticInputNodeFactory::ShaderSemanticInputNodeFactory(eUniformSemantic semantic)
	: m_semantic(semantic)
{
}

std::string ShaderSemanticInputNodeFactory::getFactoryKey() const
{
	return ShaderSemanticInputNode::k_nodeClassName + ":" + getUniformSemanticName(m_semantic);
}

std::string ShaderSemanticInputNodeFactory::editorGetCategory() const { return locText("nodes.categoryInputs"); }

NodePtr ShaderSemanticInputNodeFactory::allocateNode() const
{
	auto node= std::make_shared<ShaderSemanticInputNode>();
	node->setSemantic(m_semantic);

	return node;
}

NodePtr ShaderSemanticInputNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderSemanticInputNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr outputPin=
		node->addShaderOutputPin(ShaderSemanticInputNode::k_valuePinName, node->getValueType());

	autoConnectOutputPin(editorState, outputPin);

	return node;
}
