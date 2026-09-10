#include "ShaderRerouteNodes.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "MaterialCompiler/MaterialCompiler.h"

#include "imgui.h"

#include <cstring>
#include <vector>

namespace
{
const float k_rerouteUsageDropOffset= 140.f;

// Every declaration on one page, in id order, for the usage node's picker
std::vector<ShaderRerouteDeclarationNodePtr> collectRerouteDeclarations(NodeGraphPtr graph, t_graph_page_id pageId)
{
	std::vector<ShaderRerouteDeclarationNodePtr> declarations;
	if (!graph)
		return declarations;

	for (const auto& [nodeId, node] : graph->getNodesMap())
	{
		auto declaration= std::dynamic_pointer_cast<ShaderRerouteDeclarationNode>(node);
		if (declaration && declaration->getPageId() == pageId)
		{
			declarations.push_back(declaration);
		}
	}

	return declarations;
}
} // namespace

// -- ShaderRerouteDeclarationNodeConfig -----
configuru::Config ShaderRerouteDeclarationNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["name"]= name;

	return pt;
}

void ShaderRerouteDeclarationNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	name= pt.get_or<std::string>("name", "");
}

// -- ShaderRerouteDeclarationNode -----
bool ShaderRerouteDeclarationNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderRerouteDeclarationNodeConfig>(nodeConfig);

		m_name= config->name;

		return true;
	}

	return false;
}

void ShaderRerouteDeclarationNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderRerouteDeclarationNodeConfig>(nodeConfig);

	config->name= m_name;

	Node::saveToConfig(nodeConfig);
}

bool ShaderRerouteDeclarationNode::compileNode(MaterialCompileContext& context)
{
	// A pass-through: the usages read the same input pin directly
	const ShaderValue value= context.input(k_valuePinName);
	if (!value.isValid())
		return false;

	context.setOutput(k_valuePinName, value);

	return true;
}

std::string ShaderRerouteDeclarationNode::editorGetTitle() const
{
	return !m_name.empty() ? m_name : locText("nodes.rerouteDeclarationTitle");
}

const char* ShaderRerouteDeclarationNode::editorGetHeaderIcon() const { return ICON_FK_TAG; }

ImVec4 ShaderRerouteDeclarationNode::editorGetHeaderColor() const
{
	return ImVec4(96.f / 255.f, 140.f / 255.f, 120.f / 255.f, 225.f / 255.f);
}

void ShaderRerouteDeclarationNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.rerouteDeclarationHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		char nameBuffer[128];
		strncpy_s(nameBuffer, sizeof(nameBuffer), m_name.c_str(), _TRUNCATE);
		if (MkGui::drawStringProperty(propertyStyle, "rerouteName", locText("nodes.rerouteName"), nameBuffer,
									  sizeof(nameBuffer)))
		{
			m_name= nameBuffer;
		}
	}
}

void ShaderRerouteDeclarationNode::editorOnDoubleClicked(const NodeEditorState& editorState)
{
	NodeGraphPtr graph= getOwnerGraph();
	if (!graph)
		return;

	// A usage lands just below the declaration, already bound to it
	NodeEditorState usageState= editorState;
	usageState.currentPageId= getPageId();
	usageState.hangPosGridSpace= ImVec2(m_nodePos.x, m_nodePos.y + k_rerouteUsageDropOffset);
	auto usage= graph->createTypedNode<ShaderRerouteUsageNode>(usageState);
	if (usage)
	{
		usage->setDeclarationId(getId());
	}
}

// -- ShaderRerouteDeclarationNodeFactory -----
std::string ShaderRerouteDeclarationNodeFactory::editorGetCategory() const { return locText("nodes.categoryUtility"); }

NodePtr ShaderRerouteDeclarationNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderRerouteDeclarationNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr inputPin=
		node->addShaderInputPin(ShaderRerouteDeclarationNode::k_valuePinName, eShaderValueType::wildcard);
	ShaderValuePinPtr outputPin=
		node->addShaderOutputPin(ShaderRerouteDeclarationNode::k_valuePinName, eShaderValueType::wildcard);

	autoConnectInputPin(editorState, inputPin);
	autoConnectOutputPin(editorState, outputPin);

	return node;
}

// -- ShaderRerouteUsageNodeConfig -----
configuru::Config ShaderRerouteUsageNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["declaration_id"]= declarationId;

	return pt;
}

void ShaderRerouteUsageNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	declarationId= pt.get_or<t_node_id>("declaration_id", -1);
}

// -- ShaderRerouteUsageNode -----
bool ShaderRerouteUsageNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderRerouteUsageNodeConfig>(nodeConfig);

		m_declarationId= config->declarationId;

		return true;
	}

	return false;
}

void ShaderRerouteUsageNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderRerouteUsageNodeConfig>(nodeConfig);

	config->declarationId= m_declarationId;

	Node::saveToConfig(nodeConfig);
}

ShaderRerouteDeclarationNodePtr ShaderRerouteUsageNode::getDeclaration() const
{
	NodeGraphPtr graph= getOwnerGraph();
	if (!graph || m_declarationId == -1)
		return ShaderRerouteDeclarationNodePtr();

	return std::dynamic_pointer_cast<ShaderRerouteDeclarationNode>(graph->getNodeById(m_declarationId));
}

bool ShaderRerouteUsageNode::compileNode(MaterialCompileContext& context)
{
	ShaderRerouteDeclarationNodePtr declaration= getDeclaration();
	if (!declaration)
	{
		context.error("Reroute usage has no declaration");
		return false;
	}

	if (declaration->getPageId() != getPageId())
	{
		context.error("Reroute usage and its declaration sit on different pages");
		return false;
	}

	// The usage is the declaration's input read from elsewhere: no temp, no wire
	const ShaderValue value=
		context.input(declaration->getShaderInputPin(ShaderRerouteDeclarationNode::k_valuePinName));
	if (!value.isValid())
		return false;

	context.setOutput(k_valuePinName, value);

	return true;
}

std::string ShaderRerouteUsageNode::editorGetTitle() const
{
	ShaderRerouteDeclarationNodePtr declaration= getDeclaration();
	if (declaration && !declaration->getRerouteName().empty())
		return declaration->getRerouteName();

	return locText("nodes.rerouteUsageTitle");
}

const char* ShaderRerouteUsageNode::editorGetHeaderIcon() const { return ICON_FK_EXTERNAL_LINK; }

ImVec4 ShaderRerouteUsageNode::editorGetHeaderColor() const
{
	return ImVec4(96.f / 255.f, 140.f / 255.f, 120.f / 255.f, 225.f / 255.f);
}

void ShaderRerouteUsageNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.rerouteUsageHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

		// Pick among the declarations on this page
		const std::vector<ShaderRerouteDeclarationNodePtr> declarations=
			collectRerouteDeclarations(getOwnerGraph(), getPageId());
		std::string items;
		int selectedIndex= -1;
		for (size_t index= 0; index < declarations.size(); ++index)
		{
			items+= declarations[index]->editorGetTitle();
			items+= '\0';
			if (declarations[index]->getId() == m_declarationId)
			{
				selectedIndex= (int)index;
			}
		}
		items+= '\0';

		if (MkGui::drawSimpleComboBoxProperty(propertyStyle, "rerouteDeclaration", locText("nodes.rerouteDeclaration"),
											  items.c_str(), selectedIndex))
		{
			if (selectedIndex >= 0 && selectedIndex < (int)declarations.size())
			{
				m_declarationId= declarations[selectedIndex]->getId();
			}
		}
	}
}

// -- ShaderRerouteUsageNodeFactory -----
std::string ShaderRerouteUsageNodeFactory::editorGetCategory() const { return locText("nodes.categoryUtility"); }

NodePtr ShaderRerouteUsageNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderRerouteUsageNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr outputPin=
		node->addShaderOutputPin(ShaderRerouteUsageNode::k_valuePinName, eShaderValueType::wildcard);

	// A fresh usage binds to the page's only declaration when there is exactly one
	const std::vector<ShaderRerouteDeclarationNodePtr> declarations=
		collectRerouteDeclarations(editorState.nodeGraph, editorState.currentPageId);
	if (declarations.size() == 1)
	{
		node->setDeclarationId(declarations.front()->getId());
	}

	autoConnectOutputPin(editorState, outputPin);

	return node;
}
