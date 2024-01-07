#include "VariableNode.h"
#include "Logger.h"
#include "NodeEditorState.h"
#include "Graphs/NodeGraph.h"
#include "Graphs/NodeEvaluator.h"
#include "NodeEditorUI.h"
#include "StringUtils.h"

#include "Pins/FlowPin.h"
#include "Pins/NodePin.h"
#include "Pins/ValuePin.h"

#include "Properties/GraphValueProperty.h"

#include "imgui.h"
#include "imnodes.h"

const std::string g_variableEvalModeStrings[(int)eVariableEvalMode::COUNT] = {
	"get",
	"set"
};
extern const std::string* k_variableEvalModeStrings = g_variableEvalModeStrings;

// -- ValueSourceComboDataSource ---
class ValueSourceComboDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	ValueSourceComboDataSource(VariableNodePtr ownerNode)
	{
		auto ownerGraph = ownerNode->getOwnerGraph();
		int listIndex = 0;

		currentValueSource = ownerNode->getValueSource();

		for (auto it= ownerGraph->getPropertyMap().begin(); 
			 it != ownerGraph->getPropertyMap().end();
			 it++)
		{
			auto valueSource = std::dynamic_pointer_cast<GraphValueProperty>(it->second);

			if (valueSource)
			{
				if (valueSource == currentValueSource)
				{
					selectedValueSourceIndex = listIndex;
				}

				comboEntries.push_back({ valueSource, valueSource->getName() });
				listIndex++;
			}
		}
	}

	inline int getCurrentValueSourceIndex() const
	{
		return selectedValueSourceIndex;
	}

	inline GraphValuePropertyPtr getEntryValueSource(int index)
	{
		return comboEntries[index].valueSource;
	}

	virtual int getEntryCount() override
	{
		return (int)comboEntries.size();
	}

	virtual const std::string& getEntryDisplayString(int index) override
	{
		return comboEntries[index].entryString;
	}

private:
	struct ComboEntry
	{
		GraphValuePropertyPtr valueSource;
		std::string entryString;
	};

	GraphValuePropertyPtr currentValueSource;
	std::vector<ComboEntry> comboEntries;
	int selectedValueSourceIndex = -1;
};

// -- VariableNodeConfig -----
configuru::Config VariableNodeConfig::writeToJSON()
{
	configuru::Config pt = NodeConfig::writeToJSON();

	pt["value_property_id"] = valuePropertyId;
	pt["pin_mode"] = k_variableEvalModeStrings[(int)evalMode];

	return pt;
}

void VariableNodeConfig::readFromJSON(const configuru::Config& pt)
{
	NodeConfig::readFromJSON(pt);

	valuePropertyId = pt.get_or<t_graph_property_id>("value_property_id", -1);

	const std::string pinModeString =
		pt.get_or<std::string>(
			"pin_mode",
			k_variableEvalModeStrings[(int)eVariableEvalMode::get]);
	evalMode =
		StringUtils::FindEnumValue<eVariableEvalMode>(
			pinModeString, k_variableEvalModeStrings);
}

// -- VariableNode -----
VariableNode::~VariableNode()
{
	setOwnerGraph(NodeGraphPtr());
}

void VariableNode::setOwnerGraph(NodeGraphPtr newOwnerGraph)
{
	if (newOwnerGraph != m_ownerGraph)
	{
		if (m_ownerGraph)
		{
			m_ownerGraph->OnPropertyDeleted -= MakeDelegate(this, &VariableNode::onGraphPropertyDeleted);
			m_ownerGraph = nullptr;
		}

		if (newOwnerGraph)
		{
			newOwnerGraph->OnPropertyDeleted += MakeDelegate(this, &VariableNode::onGraphPropertyDeleted);
			m_ownerGraph = newOwnerGraph;
		}
	}
}

bool VariableNode::loadFromConfig(NodeConfigConstPtr nodeConfig)
{
	if (Node::loadFromConfig(nodeConfig))
	{
		auto variableNodeConfig = std::static_pointer_cast<const VariableNodeConfig>(nodeConfig);

		t_graph_property_id propId = variableNodeConfig->valuePropertyId;
		auto valueProperty = getOwnerGraph()->getTypedPropertyById<GraphValueProperty>(propId);
		if (valueProperty)
		{
			m_sourceProperty= valueProperty;
			return true;
		}
		else
		{
			MIKAN_LOG_WARNING("VariableNode::loadFromConfig")
				<< "Failed to find value property: " << propId
				<< ", on Variable node";
		}

		// Determines if we are getting or setting the property value
		m_evalMode = variableNodeConfig->evalMode;
	}

	return false;
}

void VariableNode::editorRenderPropertySheet(const NodeEditorState& editorState)
{
	// title bar
	if (NodeEditorUI::DrawPropertySheetHeader("Variable Node"))
	{
		bool bPinsNeedRebuild = false;

		// Name
		const std::string property_name = m_sourceProperty ? m_sourceProperty->getName() : "<INVALID>";
		NodeEditorUI::DrawStaticTextProperty("Name", property_name);

		// Evaluation Mode
		int iEvalMode = (int)m_evalMode;
		if (NodeEditorUI::DrawSimpleComboBoxProperty(
			"variableNodeEvalMode",
			"Eval Mode",
			"Get\0Set\0",
			iEvalMode))
		{
			m_evalMode = (eVariableEvalMode)iEvalMode;
			bPinsNeedRebuild= true;
		}

		// Value
		auto self= std::static_pointer_cast<VariableNode>(shared_from_this());
		ValueSourceComboDataSource dataSource(self);
		int valueSourceIndex = dataSource.getCurrentValueSourceIndex();
		if (NodeEditorUI::DrawComboBoxProperty(
			"variableSource",
			"Source",
			&dataSource,
			valueSourceIndex))
		{
			m_sourceProperty= dataSource.getEntryValueSource(valueSourceIndex);
			bPinsNeedRebuild= true;
		}

		if (bPinsNeedRebuild)
		{
			rebuildPins();
		}
	}
}

void VariableNode::saveToConfig(NodeConfigPtr nodeConfig) const
{
	auto variableNodeConfig = std::static_pointer_cast<VariableNodeConfig>(nodeConfig);
	variableNodeConfig->valuePropertyId = m_sourceProperty ? m_sourceProperty->getId() : -1;
	variableNodeConfig->evalMode = m_evalMode;

	Node::saveToConfig(nodeConfig);
}

void VariableNode::setValueSource(GraphValuePropertyPtr inValueProperty)
{
	if (m_sourceProperty != inValueProperty)
	{
		m_sourceProperty= inValueProperty;
		rebuildPins();
	}
}

bool VariableNode::evaluateNode(NodeEvaluator& evaluator)
{
	bool bSuccess= true;

	if (!m_sourceProperty)
	{
		evaluator.setLastErrorCode(eNodeEvaluationErrorCode::evaluationError);
		evaluator.setLastErrorMessage("Missing source property");
		bSuccess = false;
	}

	if (m_evalMode == eVariableEvalMode::set)
	{
		if (bSuccess)
		{
			// Only evaluate the input pin if we are in "Set" eval mode
			bSuccess = evaluateInputs(evaluator);
		}

		if (bSuccess)
		{
			// Copy value from the input pin to the property
			if (m_inputValuePin)
			{
				m_sourceProperty->copyValueFromPin(m_inputValuePin);
			}
			else
			{
				evaluator.setLastErrorCode(eNodeEvaluationErrorCode::evaluationError);
				evaluator.setLastErrorMessage("Missing input pin");
				bSuccess= false;
			}
		}
	}

	if (bSuccess)
	{
		// Copy value from the property to the output pin
		if (m_outputValuePin)
		{
			m_sourceProperty->copyValueToPin(m_outputValuePin);
		}
		else
		{
			evaluator.setLastErrorCode(eNodeEvaluationErrorCode::evaluationError);
			evaluator.setLastErrorMessage("Missing output pin");
			bSuccess = false;
		}
	}

	return bSuccess;
}

void VariableNode::editorRenderPushNodeStyle(const NodeEditorState& editorState) const
{
	if (m_outputValuePin)
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, m_outputValuePin->editorValuePinColor(1.f));
	}
	else
	{
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, ImGui::GetColorU32(NodeEditorUI::getPropertyColor(1.f)));
	}
	ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
	ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));
}

std::string VariableNode::editorGetTitle() const
{
	return m_sourceProperty ? m_sourceProperty->getName() : "Unbound Variable";
}


void VariableNode::onGraphPropertyDeleted(t_graph_property_id id)
{
	if (m_sourceProperty && m_sourceProperty->getId() == id)
	{
		setValueSource(GraphValuePropertyPtr());
	}
}

void VariableNode::rebuildPins()
{
	m_inputValuePin = nullptr;
	m_outputValuePin = nullptr;

	// Remove all existing pins based on any prior value source
	disconnectAllPins();

	if (m_sourceProperty)
	{
		if (m_evalMode == eVariableEvalMode::set)
		{
			// Variable nodes in set mode need a flow in and out pin
			// so that we control the order in which the value is set
			addPin<FlowPin>("flowIn", eNodePinDirection::INPUT);
			addPin<FlowPin>("flowOut", eNodePinDirection::OUTPUT);

			// Create the input value pin
			m_inputValuePin =
				std::dynamic_pointer_cast<ValuePin>(
					addPinByClassName(m_sourceProperty->getPinClassName(), "valueIn", eNodePinDirection::INPUT));
			assert(m_inputValuePin);
			m_inputValuePin->editorSetShowPinName(false);
		}

		// Both Set and Get modes create an output value pin
		m_outputValuePin= 
			std::dynamic_pointer_cast<ValuePin>(
				addPinByClassName(m_sourceProperty->getPinClassName(), "valueOut", eNodePinDirection::OUTPUT));
		assert(m_outputValuePin);
		m_outputValuePin->editorSetShowPinName(false);
	}
}

// -- VariableNode Factory -----
NodePtr VariableNodeFactory::createNode(const NodeEditorState& editorState) const
{
	// Create the node
	NodePtr node = NodeFactory::createNode(editorState);

	// Defer pin creation until a setValueSource() is called

	return node;
}