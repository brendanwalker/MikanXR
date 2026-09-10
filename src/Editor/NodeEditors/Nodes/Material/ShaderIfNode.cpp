#include "ShaderIfNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

namespace
{
// Indexed by eShaderCompareOp
const std::string k_shaderCompareNames[]= {"less", "lessEqual", "greater", "greaterEqual", "equal", "notEqual"};
const char* k_shaderCompareTitleKeys[]= {"nodes.ifLessTitle",    "nodes.ifLessEqualTitle",
										 "nodes.ifGreaterTitle", "nodes.ifGreaterEqualTitle",
										 "nodes.ifEqualTitle",   "nodes.ifNotEqualTitle"};
const int k_shaderCompareCount= 6;

bool isEqualityComparison(eShaderCompareOp comparison)
{
	return comparison == eShaderCompareOp::equal || comparison == eShaderCompareOp::notEqual;
}
} // namespace

// -- ShaderIfNodeConfig -----
configuru::Config ShaderIfNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["comparison"]= comparison;

	return pt;
}

void ShaderIfNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	comparison= pt.get_or<std::string>("comparison", ShaderIfNode::comparisonToName(eShaderCompareOp::less));
}

// -- ShaderIfNode -----
bool ShaderIfNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderIfNodeConfig>(nodeConfig);

		// The pins are restored by the graph loader, so only the comparison is read here
		m_comparison= comparisonFromName(config->comparison);

		return true;
	}

	return false;
}

void ShaderIfNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderIfNodeConfig>(nodeConfig);

	config->comparison= comparisonToName(m_comparison);

	Node::saveToConfig(nodeConfig);
}

bool ShaderIfNode::compileNode(MaterialCompileContext& context)
{
	const IShaderWriter& writer= context.getWriter();

	ShaderValue a= context.input(k_aPinName);
	ShaderValue b= context.input(k_bPinName);
	ShaderValue whenTrue= context.input(k_whenTruePinName);
	ShaderValue whenFalse= context.input(k_whenFalsePinName);
	if (!a.isValid() || !b.isValid() || !whenTrue.isValid() || !whenFalse.isValid())
		return false;

	// The compared operands share one type. Ordering only exists for scalars.
	const eShaderValueType compareType= ShaderValueTypeUtils::promote(a.type, b.type);
	if (compareType == eShaderValueType::INVALID)
	{
		context.error("Compared types " + ShaderValueTypeUtils::toString(a.type) + " and "
					  + ShaderValueTypeUtils::toString(b.type) + " do not combine");
		return false;
	}
	if (!isEqualityComparison(m_comparison) && compareType != eShaderValueType::float1)
	{
		context.error("An ordering comparison needs scalar operands, not "
					  + ShaderValueTypeUtils::toString(compareType));
		return false;
	}
	a= context.coerce(a, compareType);
	b= context.coerce(b, compareType);

	// The selected values share one type too
	const eShaderValueType resultType= ShaderValueTypeUtils::promote(whenTrue.type, whenFalse.type);
	if (resultType == eShaderValueType::INVALID)
	{
		context.error("Selected types " + ShaderValueTypeUtils::toString(whenTrue.type) + " and "
					  + ShaderValueTypeUtils::toString(whenFalse.type) + " do not combine");
		return false;
	}
	whenTrue= context.coerce(whenTrue, resultType);
	whenFalse= context.coerce(whenFalse, resultType);
	if (!a.isValid() || !b.isValid() || !whenTrue.isValid() || !whenFalse.isValid())
		return false;

	const std::string condition= writer.compare(m_comparison, a.expr, b.expr);
	context.setOutput(k_resultPinName,
					  context.emitTemp(resultType, writer.select(condition, whenTrue.expr, whenFalse.expr)));

	return true;
}

std::string ShaderIfNode::editorGetTitle() const
{
	const int index= (int)m_comparison;
	return locText((index >= 0 && index < k_shaderCompareCount) ? k_shaderCompareTitleKeys[index]
																: k_shaderCompareTitleKeys[0]);
}

const char* ShaderIfNode::editorGetHeaderIcon() const { return ICON_FK_CODE_FORK; }

const std::string& ShaderIfNode::comparisonToName(eShaderCompareOp comparison)
{
	const int index= (int)comparison;
	return k_shaderCompareNames[(index >= 0 && index < k_shaderCompareCount) ? index : 0];
}

eShaderCompareOp ShaderIfNode::comparisonFromName(const std::string& name)
{
	for (int index= 0; index < k_shaderCompareCount; ++index)
	{
		if (k_shaderCompareNames[index] == name)
			return (eShaderCompareOp)index;
	}

	return eShaderCompareOp::less;
}

// -- ShaderIfNodeFactory -----
ShaderIfNodeFactory::ShaderIfNodeFactory(eShaderCompareOp comparison)
	: m_comparison(comparison)
{
}

std::string ShaderIfNodeFactory::getFactoryKey() const
{
	return ShaderIfNode::k_nodeClassName + ":" + ShaderIfNode::comparisonToName(m_comparison);
}

std::string ShaderIfNodeFactory::editorGetCategory() const { return locText("nodes.categoryMath"); }

NodePtr ShaderIfNodeFactory::allocateNode() const
{
	auto node= std::make_shared<ShaderIfNode>();
	node->setComparison(m_comparison);

	return node;
}

NodePtr ShaderIfNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderIfNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr aPin= node->addShaderInputPin(ShaderIfNode::k_aPinName, eShaderValueType::wildcard);
	node->addShaderInputPin(ShaderIfNode::k_bPinName, eShaderValueType::wildcard);
	node->addShaderInputPin(ShaderIfNode::k_whenTruePinName, eShaderValueType::wildcard, {1.f, 1.f, 1.f, 1.f});
	node->addShaderInputPin(ShaderIfNode::k_whenFalsePinName, eShaderValueType::wildcard);
	ShaderValuePinPtr resultPin= node->addShaderOutputPin(ShaderIfNode::k_resultPinName, eShaderValueType::wildcard);

	autoConnectInputPin(editorState, aPin);
	autoConnectOutputPin(editorState, resultPin);

	return node;
}
