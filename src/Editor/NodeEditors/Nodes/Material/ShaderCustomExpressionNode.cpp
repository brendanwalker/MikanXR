#include "ShaderCustomExpressionNode.h"
#include "ShaderNodeUtils.h"
#include "IconsForkAwesome.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "StringUtils.h"
#include "Graphs/NodeGraph.h"
#include "Pins/NodeLink.h"
#include "MaterialCompiler/MaterialCompiler.h"

#include "imgui.h"

#include <algorithm>
#include <cfloat>
#include <cstring>

namespace
{
const char* k_customExpressionInputNamePrefix= "input";
const float k_customExpressionBodyLineCount= 12.f;

eShaderValueType parseCustomExpressionType(const std::string& typeName, eShaderValueType fallback)
{
	const eShaderValueType type= ShaderValueTypeUtils::fromString(typeName);
	return ShaderValueTypeUtils::isFloatVector(type) ? type : fallback;
}
} // namespace

// -- ShaderCustomExpressionNodeConfig -----
configuru::Config ShaderCustomExpressionNodeConfig::writeToJSON()
{
	configuru::Config pt= NodeConfig::writeToJSON();

	pt["code"]= code;
	pt["output_type"]= outputType;

	configuru::Config inputsArray= configuru::Config::array();
	for (const ShaderCustomExpressionInputConfig& input : inputs)
	{
		configuru::Config entry= configuru::Config::object();
		entry["name"]= input.name;
		entry["value_type"]= input.valueType;
		inputsArray.push_back(entry);
	}
	pt["inputs"]= inputsArray;

	return pt;
}

void ShaderCustomExpressionNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	code= pt.get_or<std::string>("code", "");
	outputType= pt.get_or<std::string>("output_type", ShaderValueTypeUtils::toString(eShaderValueType::float4));

	inputs.clear();
	if (pt.has_key("inputs") && pt["inputs"].is_array())
	{
		for (const configuru::Config& entry : pt["inputs"].as_array())
		{
			ShaderCustomExpressionInputConfig input;
			input.name= entry.get_or<std::string>("name", "");
			input.valueType=
				entry.get_or<std::string>("value_type", ShaderValueTypeUtils::toString(eShaderValueType::float1));
			inputs.push_back(input);
		}
	}
}

// -- ShaderCustomExpressionNode -----
bool ShaderCustomExpressionNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto config= std::static_pointer_cast<const ShaderCustomExpressionNodeConfig>(nodeConfig);

		// The pins are restored by the graph loader, so only the declarations are read here
		setCode(config->code);
		m_outputType= parseCustomExpressionType(config->outputType, eShaderValueType::float4);

		m_inputs.clear();
		for (const ShaderCustomExpressionInputConfig& inputConfig : config->inputs)
		{
			ShaderCustomExpressionInput input;
			input.name= inputConfig.name;
			input.type= parseCustomExpressionType(inputConfig.valueType, eShaderValueType::float1);
			m_inputs.push_back(input);
		}

		return true;
	}

	return false;
}

void ShaderCustomExpressionNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto config= std::static_pointer_cast<ShaderCustomExpressionNodeConfig>(nodeConfig);

	config->code= m_code;
	config->outputType= ShaderValueTypeUtils::toString(m_outputType);

	config->inputs.clear();
	for (const ShaderCustomExpressionInput& input : m_inputs)
	{
		ShaderCustomExpressionInputConfig inputConfig;
		inputConfig.name= input.name;
		inputConfig.valueType= ShaderValueTypeUtils::toString(input.type);
		config->inputs.push_back(inputConfig);
	}

	Node::saveToConfig(nodeConfig);
}

void ShaderCustomExpressionNode::setCode(const std::string& code)
{
	// The body editor is a fixed buffer, so the code never grows past what it can hold
	m_code= code.substr(0, k_codeBufferSize - 1);
}

void ShaderCustomExpressionNode::setOutputType(eShaderValueType outputType)
{
	if (!ShaderValueTypeUtils::isFloatVector(outputType) || outputType == m_outputType)
		return;

	m_outputType= outputType;
	retypePin(getShaderOutputPin(k_resultPinName), m_outputType);
}

void ShaderCustomExpressionNode::addInput()
{
	ShaderCustomExpressionInput input;
	input.name= makeUniqueInputName();
	input.type= eShaderValueType::float1;
	m_inputs.push_back(input);

	rebuildInputPins();
}

void ShaderCustomExpressionNode::removeInput(size_t index)
{
	if (index >= m_inputs.size())
		return;

	m_inputs.erase(m_inputs.begin() + index);

	rebuildInputPins();
}

void ShaderCustomExpressionNode::renameInput(size_t index, const std::string& name)
{
	if (index >= m_inputs.size() || m_inputs[index].name == name)
		return;

	if (!ShaderNodeUtils::isValidIdentifier(name) || hasInputNamed(name))
		return;

	// Pins match declarations by name, so a rename replaces the pin and drops its links
	m_inputs[index].name= name;

	rebuildInputPins();
}

void ShaderCustomExpressionNode::setInputType(size_t index, eShaderValueType type)
{
	if (index >= m_inputs.size() || !ShaderValueTypeUtils::isFloatVector(type) || m_inputs[index].type == type)
		return;

	m_inputs[index].type= type;

	rebuildInputPins();
}

bool ShaderCustomExpressionNode::hasInputNamed(const std::string& name) const
{
	return std::any_of(m_inputs.begin(), m_inputs.end(),
					   [&name](const ShaderCustomExpressionInput& input) { return input.name == name; });
}

std::string ShaderCustomExpressionNode::makeUniqueInputName() const
{
	for (int suffix= 0;; ++suffix)
	{
		const std::string candidate= StringUtils::stringify(k_customExpressionInputNamePrefix, suffix);
		if (!hasInputNamed(candidate))
			return candidate;
	}
}

void ShaderCustomExpressionNode::retypePin(ShaderValuePinPtr pin, eShaderValueType type)
{
	if (!pin)
		return;

	pin->setDeclaredType(type);

	// Links that cannot convert to the new type are gone, the rest stay
	NodeGraphPtr ownerGraph= getOwnerGraph();
	if (!ownerGraph)
		return;

	const std::vector<NodeLinkPtr> links= pin->getConnectedLinks();
	for (NodeLinkPtr link : links)
	{
		NodePinPtr otherPin= link->getConnectedPin(pin);
		if (!otherPin || !pin->canPinsBeConnected(otherPin))
		{
			ownerGraph->deleteLinkById(link->getId());
		}
	}
}

void ShaderCustomExpressionNode::rebuildInputPins()
{
	NodeGraphPtr ownerGraph= getOwnerGraph();
	if (!ownerGraph)
		return;

	// Drop the dynamic pins that no longer have a declaration
	for (int pinIndex= (int)m_pinsIn.size() - 1; pinIndex >= 0; --pinIndex)
	{
		NodePinPtr pin= m_pinsIn[pinIndex];
		if (pin->getIsDynamicPin() && !hasInputNamed(pin->getName()))
		{
			ownerGraph->deletePinById(pin->getId());
		}
	}

	// Add the missing ones, retype the ones that stay
	for (const ShaderCustomExpressionInput& input : m_inputs)
	{
		ShaderValuePinPtr pin= getShaderInputPin(input.name);
		if (pin)
		{
			if (pin->getDeclaredType() != input.type)
			{
				retypePin(pin, input.type);
			}
		}
		else
		{
			pin= addShaderInputPin(input.name, input.type);
			pin->setIsDynamicPin(true);
		}
	}

	// Pins read in declaration order
	auto declarationIndex= [this](const NodePinPtr& pin) -> size_t
	{
		for (size_t index= 0; index < m_inputs.size(); ++index)
		{
			if (m_inputs[index].name == pin->getName())
				return index;
		}
		return m_inputs.size();
	};
	std::stable_sort(m_pinsIn.begin(), m_pinsIn.end(), [&declarationIndex](const NodePinPtr& a, const NodePinPtr& b)
					 { return declarationIndex(a) < declarationIndex(b); });
}

bool ShaderCustomExpressionNode::compileNode(MaterialCompileContext& context)
{
	// The body is written in one language and copied through verbatim
	context.setGlslOnly();

	if (m_code.empty())
	{
		context.error("Custom expression has no code");
		return false;
	}

	std::vector<ShaderFunctionParam> params;
	std::vector<ShaderValue> args;
	params.reserve(m_inputs.size());
	args.reserve(m_inputs.size());
	for (const ShaderCustomExpressionInput& input : m_inputs)
	{
		if (!ShaderNodeUtils::isValidIdentifier(input.name))
		{
			context.error("Input name " + input.name + " is not a valid identifier");
			return false;
		}

		ShaderValuePinPtr pin= getShaderInputPin(input.name);
		const ShaderValue value= context.coerce(context.input(pin), input.type);
		if (!value.isValid())
			return false;

		params.push_back({input.type, input.name});
		args.push_back(value);
	}

	// One function per node, so two nodes with the same body never collide
	const std::string functionName= "custom_" + std::to_string(getId());
	context.setOutput(k_resultPinName, context.callFunction(m_outputType, functionName, params, m_code, args));

	return true;
}

std::string ShaderCustomExpressionNode::editorGetTitle() const { return locText("nodes.customExpressionTitle"); }

const char* ShaderCustomExpressionNode::editorGetHeaderIcon() const { return ICON_FK_CODE; }

void ShaderCustomExpressionNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locText("nodes.customExpressionHeader")))
	{
		MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");
		const std::string& typeItems= ShaderNodeUtils::getFloatTypeComboItems();

		// Output type
		int outputTypeIndex= ShaderNodeUtils::getFloatTypeComboIndex(m_outputType);
		if (MkGui::drawSimpleComboBoxProperty(propertyStyle, "customExpressionOutputType", locText("nodes.outputType"),
											  typeItems.c_str(), outputTypeIndex))
		{
			setOutputType(ShaderNodeUtils::getFloatTypeFromComboIndex(outputTypeIndex));
		}

		// Inputs: a name, a type and a remove button per row, then an add button
		ImGui::TextUnformatted(locText("nodes.inputs"));
		const float removeButtonWidth= ImGui::GetFrameHeight();
		const float typeComboWidth= ImGui::GetFontSize() * 5.f;
		for (size_t inputIndex= 0; inputIndex < m_inputs.size(); ++inputIndex)
		{
			ImGui::PushID((int)inputIndex);

			char nameBuffer[64];
			strncpy_s(nameBuffer, sizeof(nameBuffer), m_inputs[inputIndex].name.c_str(), _TRUNCATE);
			const float nameWidth= ImGui::GetContentRegionAvail().x - typeComboWidth - removeButtonWidth
								   - ImGui::GetStyle().ItemSpacing.x * 2.f;
			ImGui::SetNextItemWidth(std::max(nameWidth, ImGui::GetFontSize() * 4.f));
			if (ImGui::InputText("##name", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				renameInput(inputIndex, nameBuffer);
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

			ImGui::PopID();

			if (bRemove)
			{
				removeInput(inputIndex);
				break;
			}
		}

		if (MkGui::drawGlyphButtonWithLabel("customExpressionAddInput", ICON_FK_PLUS, locText("nodes.addInput")))
		{
			addInput();
		}

		// Body
		ImGui::TextUnformatted(locText("nodes.expressionBody"));
		strncpy_s(m_codeBuffer, sizeof(m_codeBuffer), m_code.c_str(), _TRUNCATE);
		const ImVec2 bodySize(-FLT_MIN, ImGui::GetTextLineHeight() * k_customExpressionBodyLineCount);
		if (ImGui::InputTextMultiline("##customExpressionCode", m_codeBuffer, sizeof(m_codeBuffer), bodySize,
									  ImGuiInputTextFlags_AllowTabInput))
		{
			m_code= m_codeBuffer;
		}
	}
}

// -- ShaderCustomExpressionNodeFactory -----
std::string ShaderCustomExpressionNodeFactory::editorGetCategory() const { return locText("nodes.categoryCustom"); }

NodePtr ShaderCustomExpressionNodeFactory::createNode(const NodeEditorState& editorState) const
{
	auto node= std::static_pointer_cast<ShaderCustomExpressionNode>(NodeFactory::createNode(editorState));

	ShaderValuePinPtr resultPin=
		node->addShaderOutputPin(ShaderCustomExpressionNode::k_resultPinName, node->getOutputType());

	autoConnectOutputPin(editorState, resultPin);

	return node;
}
