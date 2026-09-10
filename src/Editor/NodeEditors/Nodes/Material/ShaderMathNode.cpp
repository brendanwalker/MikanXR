#include "ShaderMathNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

// -- ShaderMathNodeConfig -----
configuru::Config ShaderMathNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["op"]= op;

	return pt;
}

void ShaderMathNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	op= pt.get_or<std::string>("op", ShaderMathOpTable::getOpName(eShaderMathOp::add));
}

// -- ShaderMathNode -----
bool ShaderMathNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderMathNodeConfig>(nodeConfig);

		// The pins are restored by the graph loader, so only the op is read here
		const eShaderMathOp op= ShaderMathOpTable::opFromName(config->op);
		m_op= (op != eShaderMathOp::INVALID) ? op : eShaderMathOp::add;

		return true;
	}

	return false;
}

void ShaderMathNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderMathNodeConfig>(nodeConfig);

	config->op= ShaderMathOpTable::getOpName(m_op);

	Node::saveToConfig(nodeConfig);
}

bool ShaderMathNode::compileNode(MaterialCompileContext& context)
{
	const IShaderWriter& writer= context.getWriter();
	const ShaderMathOpInfo& info= getOpInfo();

	if (info.op == eShaderMathOp::INVALID || info.inputs.empty())
	{
		context.error("Math node has no operation");
		return false;
	}

	std::vector<ShaderValue> operands;
	operands.reserve(info.inputs.size());
	for (const ShaderMathOpInput& input : info.inputs)
	{
		const ShaderValue value= context.input(input.name);
		if (!value.isValid())
			return false;

		operands.push_back(value);
	}

	// The operand type every input is coerced to, and the type of the result
	eShaderValueType operandType= eShaderValueType::INVALID;
	eShaderValueType resultType= eShaderValueType::INVALID;
	switch (info.rule)
	{
	case eShaderMathTypeRule::float3Result:
		operandType= eShaderValueType::float3;
		resultType= eShaderValueType::float3;
		break;

	case eShaderMathTypeRule::promote:
	case eShaderMathTypeRule::scalarResult:
	default:
	{
		// Fold the promotion over every operand: a float1 widens to whatever
		// it meets, equal vectors pass through, unequal vectors do not combine
		operandType= operands[0].type;
		if (!ShaderValueTypeUtils::isFloatVector(operandType))
		{
			context.error("Operand type " + ShaderValueTypeUtils::toString(operandType) + " is not a float vector",
						  getShaderInputPin(info.inputs[0].name));
			return false;
		}

		for (size_t index= 1; index < operands.size(); ++index)
		{
			const eShaderValueType promoted= ShaderValueTypeUtils::promote(operandType, operands[index].type);
			if (promoted == eShaderValueType::INVALID)
			{
				context.error("Operand types " + ShaderValueTypeUtils::toString(operandType) + " and "
								  + ShaderValueTypeUtils::toString(operands[index].type) + " do not combine",
							  getShaderInputPin(info.inputs[index].name));
				return false;
			}
			operandType= promoted;
		}

		resultType= (info.rule == eShaderMathTypeRule::scalarResult) ? eShaderValueType::float1 : operandType;
	}
	break;
	}

	// Every operand, lerp's alpha included, meets the operand type (a float1 broadcasts)
	std::vector<std::string> args;
	args.reserve(operands.size());
	for (ShaderValue& operand : operands)
	{
		operand= context.coerce(operand, operandType);
		if (!operand.isValid())
			return false;

		args.push_back(operand.expr);
	}

	std::string expr;
	switch (info.emit)
	{
	case eShaderMathEmit::binaryOp:
		if (args.size() != 2)
		{
			context.error("Binary math op needs two operands");
			return false;
		}
		expr= writer.binaryOp(info.binaryOp, args[0], args[1]);
		break;

	case eShaderMathEmit::oneMinus:
		expr= writer.binaryOp(eShaderBinaryOp::subtract, writer.literal(resultType, {1.f, 1.f, 1.f, 1.f}), args[0]);
		break;

	case eShaderMathEmit::builtin:
	default:
		expr= writer.builtinCall(info.builtin, args, operandType);
		break;
	}

	context.setOutput(k_resultPinName, context.emitTemp(resultType, expr));

	return true;
}

std::string ShaderMathNode::editorGetTitle() const
{
	const ShaderMathOpInfo& info= getOpInfo();

	return !info.titleKey.empty() ? locText(info.titleKey.c_str()) : locText("nodes.nodeTitle");
}

const char* ShaderMathNode::editorGetHeaderIcon() const { return ICON_FK_CALCULATOR; }

// -- ShaderMathNodeFactory -----
ShaderMathNodeFactory::ShaderMathNodeFactory(eShaderMathOp op)
	: m_op(op)
{
}

std::string ShaderMathNodeFactory::getFactoryKey() const
{
	return ShaderMathNode::k_nodeClassName + ":" + ShaderMathOpTable::getOpName(m_op);
}

std::string ShaderMathNodeFactory::editorGetCategory() const { return locText("nodes.categoryMath"); }

NodePtr ShaderMathNodeFactory::allocateNode() const
{
	auto node= std::make_shared<ShaderMathNode>();
	node->setOp(m_op);

	return node;
}

NodePtr ShaderMathNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderMathNode>(NodeFactory::createNode(editorState));
	const ShaderMathOpInfo& info= node->getOpInfo();

	ShaderValuePinPtr firstInputPin;
	for (const ShaderMathOpInput& input : info.inputs)
	{
		const float defaultValue= input.defaultValue;
		ShaderValuePinPtr inputPin= node->addShaderInputPin(input.name, input.declaredType,
															{defaultValue, defaultValue, defaultValue, defaultValue});
		if (!firstInputPin)
		{
			firstInputPin= inputPin;
		}
	}

	ShaderValuePinPtr resultPin= node->addShaderOutputPin(ShaderMathNode::k_resultPinName, eShaderValueType::wildcard);

	if (firstInputPin)
	{
		autoConnectInputPin(editorState, firstInputPin);
	}
	autoConnectOutputPin(editorState, resultPin);

	return node;
}
