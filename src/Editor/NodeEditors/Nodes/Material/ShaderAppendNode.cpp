#include "ShaderAppendNode.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

namespace
{
const int k_appendMaxComponentCount= 4;
}

// -- ShaderAppendNode -----
bool ShaderAppendNode::compileNode(MaterialCompileContext& context)
{
	const IShaderWriter& writer= context.getWriter();

	const ShaderValue a= context.input(k_aPinName);
	if (!a.isValid())
		return false;

	const ShaderValue b= context.input(k_bPinName);
	if (!b.isValid())
		return false;

	if (!ShaderValueTypeUtils::isFloatVector(a.type))
	{
		context.error("Append needs a float vector but got " + ShaderValueTypeUtils::toString(a.type),
					  getShaderInputPin(k_aPinName));
		return false;
	}

	if (!ShaderValueTypeUtils::isFloatVector(b.type))
	{
		context.error("Append needs a float vector but got " + ShaderValueTypeUtils::toString(b.type),
					  getShaderInputPin(k_bPinName));
		return false;
	}

	const int componentCount=
		ShaderValueTypeUtils::getComponentCount(a.type) + ShaderValueTypeUtils::getComponentCount(b.type);
	if (componentCount > k_appendMaxComponentCount)
	{
		context.error("Append of " + ShaderValueTypeUtils::toString(a.type) + " and "
					  + ShaderValueTypeUtils::toString(b.type) + " exceeds four components");
		return false;
	}

	const eShaderValueType resultType= ShaderValueTypeUtils::makeFloatVector(componentCount);
	context.setOutput(k_resultPinName,
					  context.emitTemp(resultType, writer.constructVector(resultType, {a.expr, b.expr})));

	return true;
}

std::string ShaderAppendNode::editorGetTitle() const { return locText("nodes.appendTitle"); }

const char* ShaderAppendNode::editorGetHeaderIcon() const { return ICON_FK_LINK; }

// -- ShaderAppendNodeFactory -----
std::string ShaderAppendNodeFactory::editorGetCategory() const { return locText("nodes.categoryVector"); }

NodePtr ShaderAppendNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderAppendNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr aPin= node->addShaderInputPin(ShaderAppendNode::k_aPinName, eShaderValueType::wildcard);
	node->addShaderInputPin(ShaderAppendNode::k_bPinName, eShaderValueType::wildcard);
	ShaderValuePinPtr resultPin=
		node->addShaderOutputPin(ShaderAppendNode::k_resultPinName, eShaderValueType::wildcard);

	autoConnectInputPin(editorState, aPin);
	autoConnectOutputPin(editorState, resultPin);

	return node;
}
