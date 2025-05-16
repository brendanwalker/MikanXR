#include "GraphStencilProperty.h"
#include "MkMaterial.h"
#include "IMkVertexDefinition.h"
#include "Logger.h"
#include "NodeEditorUI.h"
#include "ProjectConfigConstants.h"
#include "StringUtils.h"

#include "Graphs/NodeGraph.h"

#include "Nodes/StencilNode.h"

#include "BoxStencilComponent.h"
#include "ModelStencilComponent.h"
#include "QuadStencilComponent.h"
#include "StencilObjectSystem.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- MaterialAssetComboDataSource ---
class StencilComboDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	StencilComboDataSource(
		StencilComponentPtr stencilComponent,
		eStencilType stencilType)
	{
		auto stencilSystem = StencilObjectSystem::getSystem();
		int listIndex = 0;

		// TODO: Need this overly complicated wrapper so that we can iterate over available
		// stencil names based on currently selected stencil type.
		// This should probably be a helper on the stencil object system instead.
		switch (stencilType)
		{
			case eStencilType::box:
				for (auto it = stencilSystem->getBoxStencilMap().begin();
					 it != stencilSystem->getBoxStencilMap().end();
					 it++)
				{
					auto stencilPtr = it->second.lock();
					if (stencilPtr == stencilComponent)
					{
						stencilSourceIndex = listIndex;
					}
					comboEntries.push_back({stencilPtr, stencilPtr->getName()});
					listIndex++;
				}
				break;
			case eStencilType::quad:
				for (auto it = stencilSystem->getQuadStencilMap().begin();
					 it != stencilSystem->getQuadStencilMap().end();
					 it++)
				{
					auto stencilPtr = it->second.lock();
					if (stencilPtr == stencilComponent)
					{
						stencilSourceIndex = listIndex;
					}
					comboEntries.push_back({stencilPtr, stencilPtr->getName()});
					listIndex++;
				}
				break;
			case eStencilType::model:
				for (auto it = stencilSystem->getModelStencilMap().begin();
					 it != stencilSystem->getModelStencilMap().end();
					 it++)
				{
					auto stencilPtr = it->second.lock();
					if (stencilPtr == stencilComponent)
					{
						stencilSourceIndex = listIndex;
					}
					comboEntries.push_back({stencilPtr, stencilPtr->getName()});
					listIndex++;
				}
				break;
			default:
				break;
		}
	}

	inline int getCurrentStencilIndex() const
	{
		return stencilSourceIndex;
	}

	inline StencilComponentPtr getEntryStencil(int index)
	{
		return comboEntries[index].stencil;
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
		StencilComponentPtr stencil;
		std::string entryString;
	};

	std::vector<ComboEntry> comboEntries;
	int stencilSourceIndex = -1;
};

// -- GraphStencilPropertyConfig -----
configuru::Config GraphStencilPropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	pt["stencil_type"]= 
		(stencilType != eStencilType::INVALID) 
		? k_stencilTypeStrings[(int)stencilType]
		: k_stencilTypeStrings[(int)eStencilType::quad];
	pt["stencil_name"]= stencilName;

	return pt;
}

void GraphStencilPropertyConfig::readFromJSON(const configuru::Config& pt)
{
	const std::string stencilTypeString =
		pt.get_or<std::string>(
			"stencil_type",
			k_stencilTypeStrings[(int)eStencilType::quad]);
	stencilType =
		StringUtils::FindEnumValue<eStencilType>(
			stencilTypeString,
			k_stencilTypeStrings);
	stencilName = pt.get_or<std::string>("stencil_name", "");

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphStencilProperty -----
bool GraphStencilProperty::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto& stencilPropConfig = std::static_pointer_cast<const GraphStencilPropertyConfig>(propConfig);
		if (!stencilPropConfig->stencilName.empty() && stencilPropConfig->stencilType != eStencilType::INVALID)
		{
			auto stencilSystem= StencilObjectSystem::getSystem();

			switch (stencilPropConfig->stencilType)
			{
			case eStencilType::box:
				setStencilComponent(stencilSystem->getBoxStencilByName(stencilPropConfig->stencilName));
				m_stencilType= eStencilType::box;
				break;
			case eStencilType::model:
				setStencilComponent(stencilSystem->getModelStencilByName(stencilPropConfig->stencilName));
				m_stencilType= eStencilType::model;
				break;
			case eStencilType::quad:
				setStencilComponent(stencilSystem->getQuadStencilByName(stencilPropConfig->stencilName));
				m_stencilType= eStencilType::quad;
				break;
			default:
				MIKAN_LOG_ERROR("GraphStencilProperty::loadFromConfig") 
					<< "Invalid stencil name: " << stencilPropConfig->stencilName;
				setStencilComponent(StencilComponentPtr());
				m_stencilType= eStencilType::INVALID;
			}

			return m_stencilType != eStencilType::INVALID;
		}
		else
		{
			// Config says the property had an empty stencil reference
			setStencilComponent(StencilComponentPtr());
			return true;
		}
	}

	return false;
}

void GraphStencilProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto stencilPropConfig = std::static_pointer_cast<GraphStencilPropertyConfig>(config);
	auto stencilSystem= StencilObjectSystem::getSystem();

	if (m_stencilComponent != nullptr)
	{
		StencilComponentConfigPtr definition= m_stencilComponent->getStencilComponentDefinition();

		stencilPropConfig->stencilName = definition->getComponentName();
		stencilPropConfig->id = definition->getStencilId();
		stencilPropConfig->stencilType = stencilSystem->getStencilType(stencilPropConfig->id);
	}
	else
	{
		stencilPropConfig->stencilName = "";
		stencilPropConfig->id = -1;
		stencilPropConfig->stencilType = eStencilType::INVALID;
	}

	GraphProperty::saveToConfig(config);
}

void GraphStencilProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto stencilNode = m_ownerGraph->createTypedNode<StencilNode>(editorState);

	// Set this as the source model property for the new node
	auto self = std::static_pointer_cast<GraphStencilProperty>(shared_from_this());
	stencilNode->setStencilSource(self);
}

void GraphStencilProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Stencil"))
	{
		// Name
		std::string name = m_stencilComponent ? m_stencilComponent->getName() : "<No Stencil>";
		NodeEditorUI::DrawStaticTextProperty("Name", name);

		// Stencil Type
		int stencilTypeIdex = (int)m_stencilType; 
		if (ImGui::Combo("Type", &stencilTypeIdex, k_szStencilTypeStrings, (int)eStencilType::COUNT))
		{
			setStencilComponent(StencilComponentPtr());
			m_stencilType= (eStencilType)stencilTypeIdex;
		}

		// Stencil
		StencilComboDataSource dataSource(m_stencilComponent, m_stencilType);
		int selectedIndex= dataSource.getCurrentStencilIndex();
		if (NodeEditorUI::DrawComboBoxProperty("stencilSelection", "Source", &dataSource, selectedIndex))
		{
			setStencilComponent(dataSource.getEntryStencil(selectedIndex));
		}
	}
}

const ImVec4 GraphStencilProperty::editorGetIconColor() const
{
	return NodeEditorUI::getComponentColor();
}