#include "ShaderFunctionNodes.h"
#include "ShaderNodeUtils.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "NodeEditorState.h"
#include "StringUtils.h"
#include "Graphs/MaterialFunctionPage.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodeLink.h"
#include "MaterialCompiler/MaterialCompiler.h"

#include <algorithm>

namespace
{
MaterialFunctionPagePtr findFunctionPage(NodeGraphPtr graph, t_graph_page_id pageId)
{
	return graph ? std::dynamic_pointer_cast<MaterialFunctionPage>(graph->getPageById(pageId))
				 : MaterialFunctionPagePtr();
}

// Retype a pin, dropping the links the new type can no longer carry
void retypeShaderPin(NodeGraphPtr graph, ShaderValuePinPtr pin, eShaderValueType type)
{
	if (!pin || pin->getDeclaredType() == type)
		return;

	pin->setDeclaredType(type);

	const std::vector<NodeLinkPtr> links= pin->getConnectedLinks();
	for (NodeLinkPtr link : links)
	{
		NodePinPtr otherPin= (link->getStartPin() == pin) ? link->getEndPin() : link->getStartPin();
		if (graph && otherPin && !pin->canPinsBeConnected(otherPin))
		{
			graph->deleteLinkById(link->getId());
		}
	}
}
} // namespace

// -- ShaderFunctionInputNodeConfig -----
configuru::Config ShaderFunctionInputNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["input_name"]= inputName;

	return pt;
}

void ShaderFunctionInputNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	inputName= pt.get_or<std::string>("input_name", "");
}

// -- ShaderFunctionInputNode -----
bool ShaderFunctionInputNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderFunctionInputNodeConfig>(nodeConfig);

		m_inputName= config->inputName;

		return true;
	}

	return false;
}

void ShaderFunctionInputNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderFunctionInputNodeConfig>(nodeConfig);

	config->inputName= m_inputName;

	Node::saveToConfig(nodeConfig);
}

void ShaderFunctionInputNode::syncToDeclaration(const std::string& name, eShaderValueType type)
{
	m_inputName= name;
	retypeShaderPin(getOwnerGraph(), getShaderOutputPin(k_valuePinName), type);
}

bool ShaderFunctionInputNode::compileNode(MaterialCompileContext& context)
{
	const ShaderValue value= context.functionParameter(m_inputName);
	if (!value.isValid())
		return false;

	context.setOutput(k_valuePinName, value);

	return true;
}

MaterialFunctionPagePtr ShaderFunctionInputNode::getFunctionPage() const
{
	return findFunctionPage(getOwnerGraph(), getPageId());
}

bool ShaderFunctionInputNode::editorCanDelete() const
{
	MaterialFunctionPagePtr page= getFunctionPage();

	return !page || page->findInput(m_inputName) == nullptr;
}

std::string ShaderFunctionInputNode::editorGetTitle() const
{
	return StringUtils::stringify(locText("nodes.functionInputPrefix"), m_inputName);
}

const char* ShaderFunctionInputNode::editorGetHeaderIcon() const { return ICON_FK_SIGN_IN; }

// -- ShaderFunctionOutputNode -----
void ShaderFunctionOutputNode::syncToOutputType(eShaderValueType type)
{
	retypeShaderPin(getOwnerGraph(), getValuePin(), type);
}

const char* ShaderFunctionOutputNode::editorGetHeaderIcon() const { return ICON_FK_SIGN_OUT; }

ImVec4 ShaderFunctionOutputNode::editorGetHeaderColor() const
{
	return ImVec4(150.f / 255.f, 96.f / 255.f, 110.f / 255.f, 225.f / 255.f);
}

NodePtr ShaderFunctionOutputNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderFunctionOutputNode>(NodeFactory::createNode(editorState));

	// The page retypes the pin to its output type right after creation
	node->addShaderInputPin(ShaderFunctionOutputNode::k_valuePinName, eShaderValueType::float1);

	return node;
}

// -- ShaderFunctionCallNodeConfig -----
configuru::Config ShaderFunctionCallNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["page_id"]= pageId;

	return pt;
}

void ShaderFunctionCallNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	pageId= pt.get_or<t_graph_page_id>("page_id", -1);
}

// -- ShaderFunctionCallNode -----
ShaderFunctionCallNode::~ShaderFunctionCallNode()
{
	if (m_ownerGraph)
	{
		m_ownerGraph->OnGraphLoaded-= MakeDelegate(this, &ShaderFunctionCallNode::onGraphLoaded);
		m_ownerGraph->OnPageModified-= MakeDelegate(this, &ShaderFunctionCallNode::onPageModified);
		m_ownerGraph->OnPageDeleted-= MakeDelegate(this, &ShaderFunctionCallNode::onPageDeleted);
	}
}

void ShaderFunctionCallNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnGraphLoaded-= MakeDelegate(this, &ShaderFunctionCallNode::onGraphLoaded);
			m_ownerGraph->OnPageModified-= MakeDelegate(this, &ShaderFunctionCallNode::onPageModified);
			m_ownerGraph->OnPageDeleted-= MakeDelegate(this, &ShaderFunctionCallNode::onPageDeleted);
		}

		Node::setOwnerGraph(newOwnerGraph);

		if (newOwnerGraph)
		{
			newOwnerGraph->OnGraphLoaded+= MakeDelegate(this, &ShaderFunctionCallNode::onGraphLoaded);
			newOwnerGraph->OnPageModified+= MakeDelegate(this, &ShaderFunctionCallNode::onPageModified);
			newOwnerGraph->OnPageDeleted+= MakeDelegate(this, &ShaderFunctionCallNode::onPageDeleted);
		}
	}
}

bool ShaderFunctionCallNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderFunctionCallNodeConfig>(nodeConfig);

		// The pins are restored by the graph loader and synced to the page once the graph has loaded
		m_pageId= config->pageId;

		return true;
	}

	return false;
}

void ShaderFunctionCallNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderFunctionCallNodeConfig>(nodeConfig);

	config->pageId= m_pageId;

	Node::saveToConfig(nodeConfig);
}

MaterialFunctionPagePtr ShaderFunctionCallNode::getFunctionPage() const
{
	return findFunctionPage(getOwnerGraph(), m_pageId);
}

void ShaderFunctionCallNode::rebuildPins()
{
	NodeGraphPtr ownerGraph= getOwnerGraph();
	MaterialFunctionPagePtr page= getFunctionPage();
	if (!ownerGraph || !page)
		return;

	m_functionName= page->getName();

	// Drop the dynamic pins that no longer have a declaration
	for (int pinIndex= (int)m_pinsIn.size() - 1; pinIndex >= 0; --pinIndex)
	{
		NodePinPtr pin= m_pinsIn[pinIndex];
		if (pin->getIsDynamicPin() && page->findInput(pin->getName()) == nullptr)
		{
			ownerGraph->deletePinById(pin->getId());
		}
	}

	// Add the missing ones, retype the ones that stay, and keep the declared defaults current
	for (const MaterialFunctionInput& input : page->getInputs())
	{
		ShaderValuePinPtr pin= getShaderInputPin(input.name);
		if (pin)
		{
			retypeShaderPin(ownerGraph, pin, input.type);
		}
		else
		{
			pin= addShaderInputPin(input.name, input.type, input.defaultValue);
			pin->setIsDynamicPin(true);
		}
	}

	// Pins read in declaration order
	const std::vector<MaterialFunctionInput>& inputs= page->getInputs();
	auto declarationIndex= [&inputs](const NodePinPtr& pin) -> size_t
	{
		for (size_t index= 0; index < inputs.size(); ++index)
		{
			if (inputs[index].name == pin->getName())
				return index;
		}
		return inputs.size();
	};
	std::stable_sort(m_pinsIn.begin(), m_pinsIn.end(), [&declarationIndex](const NodePinPtr& a, const NodePinPtr& b)
					 { return declarationIndex(a) < declarationIndex(b); });

	ShaderValuePinPtr resultPin= getShaderOutputPin(k_resultPinName);
	if (resultPin)
	{
		retypeShaderPin(ownerGraph, resultPin, page->getOutputType());
	}
}

bool ShaderFunctionCallNode::compileNode(MaterialCompileContext& context)
{
	MaterialFunctionPagePtr page= getFunctionPage();
	if (!page)
	{
		context.error("Function call has no function page");
		return false;
	}

	std::vector<ShaderValue> args;
	args.reserve(page->getInputs().size());
	for (const MaterialFunctionInput& input : page->getInputs())
	{
		const ShaderValue value= context.coerce(context.input(getShaderInputPin(input.name)), input.type);
		if (!value.isValid())
			return false;

		args.push_back(value);
	}

	const ShaderValue call= context.callMaterialFunction(page, args);
	if (!call.isValid())
		return false;

	context.setOutput(k_resultPinName, context.emitTemp(call.type, call.expr));

	return true;
}

std::string ShaderFunctionCallNode::editorGetTitle() const
{
	return !m_functionName.empty() ? m_functionName : locText("nodes.functionCallTitle");
}

const char* ShaderFunctionCallNode::editorGetHeaderIcon() const { return ICON_FK_PUZZLE_PIECE; }

void ShaderFunctionCallNode::onGraphLoaded(bool success)
{
	if (success)
	{
		rebuildPins();
	}
}

void ShaderFunctionCallNode::onPageModified(t_graph_page_id pageId)
{
	if (pageId == m_pageId)
	{
		rebuildPins();
	}
}

void ShaderFunctionCallNode::onPageDeleted(t_graph_page_id pageId)
{
	// The page goes with its calls; the graph fires this before deleting the page's nodes
	if (pageId == m_pageId && m_ownerGraph && !isPendingDeletion())
	{
		m_ownerGraph->deleteNodeById(getId());
	}
}

void ShaderFunctionCallNode::retypePin(ShaderValuePinPtr pin, eShaderValueType type)
{
	retypeShaderPin(getOwnerGraph(), pin, type);
}

// -- ShaderFunctionCallNodeFactory -----
ShaderFunctionCallNodeFactory::ShaderFunctionCallNodeFactory(t_graph_page_id pageId, const std::string& functionName)
	: m_pageId(pageId)
	, m_functionName(functionName)
{
}

std::string ShaderFunctionCallNodeFactory::makeFactoryKey(t_graph_page_id pageId)
{
	return ShaderFunctionCallNode::k_nodeClassName + ":" + std::to_string(pageId);
}

std::string ShaderFunctionCallNodeFactory::editorGetCategory() const { return locText("nodes.categoryFunctions"); }

NodePtr ShaderFunctionCallNodeFactory::allocateNode() const
{
	auto node= std::make_shared<ShaderFunctionCallNode>();
	node->setFunctionPageId(m_pageId);
	node->setFunctionName(m_functionName);

	return node;
}

NodePtr ShaderFunctionCallNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderFunctionCallNode>(NodeFactory::createNode(editorState));

	MaterialFunctionPagePtr page= findFunctionPage(editorState.nodeGraph, m_pageId);
	const eShaderValueType outputType= page ? page->getOutputType() : eShaderValueType::float1;
	ShaderValuePinPtr resultPin= node->addShaderOutputPin(ShaderFunctionCallNode::k_resultPinName, outputType);
	node->rebuildPins();

	autoConnectOutputPin(editorState, resultPin);

	return node;
}
