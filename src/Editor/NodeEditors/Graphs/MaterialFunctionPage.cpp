#include "MaterialFunctionPage.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "StringUtils.h"
#include "Graphs/NodeGraph.h"
#include "Nodes/Material/ShaderFunctionNodes.h"
#include "Nodes/Material/ShaderNodeUtils.h"

#include "imgui.h"

#include <algorithm>
#include <cstring>

namespace
{
const char* k_functionInputNamePrefix= "input";
const char* k_functionNamePrefix= "function";
const float k_functionInputNodeSpacing= 120.f;
} // namespace

// -- MaterialFunctionPageConfig -----
configuru::Config MaterialFunctionPageConfig::writeToJSON()
{
	configuru::Config pt= GraphPageConfig::writeToJSON();

	configuru::Config inputsArray= configuru::Config::array();
	for (const MaterialFunctionInputConfig& input : inputs)
	{
		configuru::Config entry= configuru::Config::object();
		entry["name"]= input.name;
		entry["value_type"]= input.valueType;
		entry["default_value"]= configuru::Config::array(
			{input.defaultValue[0], input.defaultValue[1], input.defaultValue[2], input.defaultValue[3]});
		inputsArray.push_back(entry);
	}
	pt["inputs"]= inputsArray;
	pt["output_type"]= outputType;

	return pt;
}

void MaterialFunctionPageConfig::readFromJSON(const configuru::Config& pt)
{
	GraphPageConfig::readFromJSON(pt);

	inputs.clear();
	if (pt.has_key("inputs") && pt["inputs"].is_array())
	{
		for (const configuru::Config& entry : pt["inputs"].as_array())
		{
			MaterialFunctionInputConfig input;
			input.name= entry.get_or<std::string>("name", "");
			input.valueType=
				entry.get_or<std::string>("value_type", ShaderValueTypeUtils::toString(eShaderValueType::float1));
			if (entry.has_key("default_value") && entry["default_value"].is_array())
			{
				const configuru::Config& valueArray= entry["default_value"];
				for (size_t index= 0; index < valueArray.array_size() && index < input.defaultValue.size(); ++index)
				{
					input.defaultValue[index]= (float)valueArray[index].as_double();
				}
			}
			inputs.push_back(input);
		}
	}
	outputType= pt.get_or<std::string>("output_type", ShaderValueTypeUtils::toString(eShaderValueType::float1));
}

// -- MaterialFunctionPage -----
bool MaterialFunctionPage::loadFromConfig(GraphPageConfigConstPtr config)
{
	if (!GraphPage::loadFromConfig(config))
		return false;

	auto pageConfig= std::static_pointer_cast<const MaterialFunctionPageConfig>(config);

	m_inputs.clear();
	for (const MaterialFunctionInputConfig& inputConfig : pageConfig->inputs)
	{
		MaterialFunctionInput input;
		input.name= inputConfig.name;
		const eShaderValueType type= ShaderValueTypeUtils::fromString(inputConfig.valueType);
		input.type= ShaderValueTypeUtils::isFloatVector(type) ? type : eShaderValueType::float1;
		input.defaultValue= inputConfig.defaultValue;
		m_inputs.push_back(input);
	}

	const eShaderValueType outputType= ShaderValueTypeUtils::fromString(pageConfig->outputType);
	m_outputType= ShaderValueTypeUtils::isFloatVector(outputType) ? outputType : eShaderValueType::float1;

	return true;
}

void MaterialFunctionPage::saveToConfig(GraphPageConfigPtr config) const
{
	GraphPage::saveToConfig(config);

	auto pageConfig= std::static_pointer_cast<MaterialFunctionPageConfig>(config);

	pageConfig->inputs.clear();
	for (const MaterialFunctionInput& input : m_inputs)
	{
		MaterialFunctionInputConfig inputConfig;
		inputConfig.name= input.name;
		inputConfig.valueType= ShaderValueTypeUtils::toString(input.type);
		inputConfig.defaultValue= input.defaultValue;
		pageConfig->inputs.push_back(inputConfig);
	}
	pageConfig->outputType= ShaderValueTypeUtils::toString(m_outputType);
}

const MaterialFunctionInput* MaterialFunctionPage::findInput(const std::string& name) const
{
	for (const MaterialFunctionInput& input : m_inputs)
	{
		if (input.name == name)
			return &input;
	}

	return nullptr;
}

bool MaterialFunctionPage::setFunctionName(const std::string& name)
{
	if (name == m_name)
		return true;

	if (!ShaderNodeUtils::isValidIdentifier(name))
		return false;

	// One function name per graph, since each becomes a shader function
	if (m_ownerGraph)
	{
		for (const auto& [pageId, page] : m_ownerGraph->getPages())
		{
			if (page.get() != this && page->getName() == name)
				return false;
		}
	}

	m_name= name;
	notifyModified();

	return true;
}

void MaterialFunctionPage::setOutputType(eShaderValueType type)
{
	if (!ShaderValueTypeUtils::isFloatVector(type) || type == m_outputType)
		return;

	m_outputType= type;
	if (ShaderFunctionOutputNodePtr outputNode= getOutputNode())
	{
		outputNode->syncToOutputType(type);
	}
	notifyModified();
}

void MaterialFunctionPage::addInput()
{
	MaterialFunctionInput input;
	input.name= makeUniqueInputName();
	input.type= eShaderValueType::float1;
	m_inputs.push_back(input);

	if (m_ownerGraph)
	{
		NodeEditorState editorState;
		editorState.nodeGraph= m_ownerGraph;
		editorState.currentPageId= m_id;
		createInputNode(input, editorState);
	}
	notifyModified();
}

void MaterialFunctionPage::removeInput(size_t index)
{
	if (index >= m_inputs.size())
		return;

	const std::string name= m_inputs[index].name;
	m_inputs.erase(m_inputs.begin() + index);

	// The declaration owned the node; it goes with its links
	if (ShaderFunctionInputNodePtr inputNode= getInputNode(name))
	{
		m_ownerGraph->deleteNodeById(inputNode->getId());
	}
	notifyModified();
}

bool MaterialFunctionPage::renameInput(size_t index, const std::string& name)
{
	if (index >= m_inputs.size() || m_inputs[index].name == name)
		return false;

	if (!ShaderNodeUtils::isValidIdentifier(name) || hasInputNamed(name))
		return false;

	ShaderFunctionInputNodePtr inputNode= getInputNode(m_inputs[index].name);
	m_inputs[index].name= name;
	if (inputNode)
	{
		inputNode->syncToDeclaration(name, m_inputs[index].type);
	}
	notifyModified();

	return true;
}

void MaterialFunctionPage::setInputType(size_t index, eShaderValueType type)
{
	if (index >= m_inputs.size() || !ShaderValueTypeUtils::isFloatVector(type) || m_inputs[index].type == type)
		return;

	m_inputs[index].type= type;
	if (ShaderFunctionInputNodePtr inputNode= getInputNode(m_inputs[index].name))
	{
		inputNode->syncToDeclaration(m_inputs[index].name, type);
	}
	notifyModified();
}

void MaterialFunctionPage::setInputDefault(size_t index, const ShaderValueDefault& value)
{
	if (index >= m_inputs.size())
		return;

	m_inputs[index].defaultValue= value;
	notifyModified();
}

ShaderFunctionOutputNodePtr MaterialFunctionPage::getOutputNode() const
{
	if (!m_ownerGraph)
		return ShaderFunctionOutputNodePtr();

	for (NodePtr node : m_ownerGraph->getNodesOnPage(m_id))
	{
		if (auto outputNode= std::dynamic_pointer_cast<ShaderFunctionOutputNode>(node))
			return outputNode;
	}

	return ShaderFunctionOutputNodePtr();
}

ShaderFunctionInputNodePtr MaterialFunctionPage::getInputNode(const std::string& inputName) const
{
	if (!m_ownerGraph)
		return ShaderFunctionInputNodePtr();

	for (NodePtr node : m_ownerGraph->getNodesOnPage(m_id))
	{
		auto inputNode= std::dynamic_pointer_cast<ShaderFunctionInputNode>(node);
		if (inputNode && inputNode->getInputName() == inputName)
			return inputNode;
	}

	return ShaderFunctionInputNodePtr();
}

void MaterialFunctionPage::onPageCreated(const NodeEditorState& editorState)
{
	// A fresh function starts with a unique identifier name and its output node
	int suffix= 1;
	std::string candidate= StringUtils::stringify(k_functionNamePrefix, suffix);
	auto isNameTaken= [this](const std::string& name)
	{
		if (!m_ownerGraph)
			return false;
		for (const auto& [pageId, page] : m_ownerGraph->getPages())
		{
			if (page.get() != this && page->getName() == name)
				return true;
		}
		return false;
	};
	while (isNameTaken(candidate))
	{
		candidate= StringUtils::stringify(k_functionNamePrefix, ++suffix);
	}
	m_name= candidate;

	NodeEditorState pageEditorState= editorState;
	pageEditorState.hangPosGridSpace= ImVec2(400.f, 100.f);
	auto outputNode= m_ownerGraph->createTypedNode<ShaderFunctionOutputNode>(pageEditorState);
	if (outputNode)
	{
		outputNode->setNodePos({400.f, 100.f});
		outputNode->syncToOutputType(m_outputType);
	}
}

void MaterialFunctionPage::onGraphLoaded()
{
	// Nothing to resolve: the nodes carry their page and the declarations their names
}

const char* MaterialFunctionPage::editorGetIcon() const { return ICON_FK_PUZZLE_PIECE; }

void MaterialFunctionPage::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("materialEditor.functionHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");
		const std::string& typeItems= ShaderNodeUtils::getFloatTypeComboItems();

		// Name (an identifier, committed on Enter so a half-typed name never lands)
		char nameBuffer[64];
		strncpy_s(nameBuffer, sizeof(nameBuffer), m_name.c_str(), _TRUNCATE);
		if (MkGui::drawStringProperty(propertyStyle, "materialFunctionName", locText("materialEditor.functionName"),
									  nameBuffer, sizeof(nameBuffer)))
		{
			setFunctionName(nameBuffer);
		}

		// Output type
		int outputTypeIndex= ShaderNodeUtils::getFloatTypeComboIndex(m_outputType);
		if (MkGui::drawSimpleComboBoxProperty(propertyStyle, "materialFunctionOutputType", locText("nodes.outputType"),
											  typeItems.c_str(), outputTypeIndex))
		{
			setOutputType(ShaderNodeUtils::getFloatTypeFromComboIndex(outputTypeIndex));
		}

		// Inputs: a name, a type and a remove button per row, the default below, then an add button
		ImGui::TextUnformatted(locText("nodes.inputs"));
		const float removeButtonWidth= ImGui::GetFrameHeight();
		const float typeComboWidth= ImGui::GetFontSize() * 5.f;
		for (size_t inputIndex= 0; inputIndex < m_inputs.size(); ++inputIndex)
		{
			ImGui::PushID((int)inputIndex);

			char inputNameBuffer[64];
			strncpy_s(inputNameBuffer, sizeof(inputNameBuffer), m_inputs[inputIndex].name.c_str(), _TRUNCATE);
			const float nameWidth= ImGui::GetContentRegionAvail().x - typeComboWidth - removeButtonWidth
								   - ImGui::GetStyle().ItemSpacing.x * 2.f;
			ImGui::SetNextItemWidth(std::max(nameWidth, ImGui::GetFontSize() * 4.f));
			if (ImGui::InputText("##name", inputNameBuffer, sizeof(inputNameBuffer),
								 ImGuiInputTextFlags_EnterReturnsTrue))
			{
				renameInput(inputIndex, inputNameBuffer);
			}

			ImGui::SameLine();
			ImGui::SetNextItemWidth(typeComboWidth);
			int typeIndex= ShaderNodeUtils::getFloatTypeComboIndex(m_inputs[inputIndex].type);
			if (ImGui::Combo("##type", &typeIndex, typeItems.c_str()))
			{
				setInputType(inputIndex, ShaderNodeUtils::getFloatTypeFromComboIndex(typeIndex));
			}

			ImGui::SameLine();
			const bool bRemove= ImGui::Button(ICON_FK_TRASH "##remove", ImVec2(removeButtonWidth, 0.f));

			ShaderValueDefault defaultValue= m_inputs[inputIndex].defaultValue;
			if (ShaderNodeUtils::drawValueProperty(propertyStyle, "materialFunctionInputDefault",
												   locText("nodes.defaultValue"), m_inputs[inputIndex].type, false,
												   defaultValue))
			{
				setInputDefault(inputIndex, defaultValue);
			}

			ImGui::PopID();

			if (bRemove)
			{
				removeInput(inputIndex);
				break;
			}
		}

		if (MkGui::drawGlyphButtonWithLabel("materialFunctionAddInput", ICON_FK_PLUS, locText("nodes.addInput")))
		{
			addInput();
		}
	}
}

bool MaterialFunctionPage::hasInputNamed(const std::string& name) const { return findInput(name) != nullptr; }

std::string MaterialFunctionPage::makeUniqueInputName() const
{
	for (int suffix= 0;; ++suffix)
	{
		const std::string candidate= StringUtils::stringify(k_functionInputNamePrefix, suffix);
		if (!hasInputNamed(candidate))
			return candidate;
	}
}

void MaterialFunctionPage::createInputNode(const MaterialFunctionInput& input, const NodeEditorState& editorState)
{
	// Inputs stack down the left edge of the page in declaration order
	const float row= (float)(m_inputs.size() - 1);
	NodeEditorState pageEditorState= editorState;
	pageEditorState.hangPosGridSpace= ImVec2(40.f, 100.f + row * k_functionInputNodeSpacing);

	auto inputNode= m_ownerGraph->createTypedNode<ShaderFunctionInputNode>(pageEditorState);
	if (inputNode)
	{
		inputNode->setInputName(input.name);
		inputNode->setNodePos({pageEditorState.hangPosGridSpace.x, pageEditorState.hangPosGridSpace.y});
		inputNode->addShaderOutputPin(ShaderFunctionInputNode::k_valuePinName, input.type);
	}
}

void MaterialFunctionPage::notifyModified()
{
	if (m_ownerGraph)
	{
		m_ownerGraph->notifyPageModified(m_id);
	}
}

// -- MaterialFunctionPageFactory -----
std::string MaterialFunctionPageFactory::editorGetCreateLabel() const { return locText("materialEditor.addFunction"); }
