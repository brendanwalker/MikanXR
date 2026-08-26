#include "GraphShapeProperty.h"
#include "Logger.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "NodeEditorState.h"
#include "ProjectConfigConstants.h"
#include "StringUtils.h"

#include "Graphs/NodeGraph.h"

#include "Nodes/ShapeNode.h"

#include "BoxShapeComponent.h"
#include "ModelShapeComponent.h"
#include "QuadShapeComponent.h"
#include "QuadShapeSystem.h"
#include "BoxShapeSystem.h"
#include "ModelShapeSystem.h"
#include "ShapeUtils.h"

#include "imgui.h"
#include "IconsForkAwesome.h"

// -- ShapeComboDataSource ---
class ShapeComboDataSource : public MkGui::ComboBoxDataSource
{
public:
	ShapeComboDataSource(NodeGraphPtr ownerGraph, ShapeComponentPtr shapeComponent, eShapeType shapeType)
	{
		if (!ownerGraph)
			return;

		int listIndex= 0;

		switch (shapeType)
		{
		case eShapeType::quad:
		{
			auto quadSystem= ownerGraph->getObjectSystemOfType<QuadShapeSystem>();
			for (auto it= quadSystem->getComponentMap().begin(); it != quadSystem->getComponentMap().end(); it++)
			{
				auto shapePtr= it->second.lock();
				if (shapePtr == shapeComponent)
					shapeSourceIndex= listIndex;
				comboEntries.push_back({shapePtr, shapePtr->getName()});
				listIndex++;
			}
		}
		break;
		case eShapeType::box:
		{
			auto boxSystem= ownerGraph->getObjectSystemOfType<BoxShapeSystem>();
			for (auto it= boxSystem->getComponentMap().begin(); it != boxSystem->getComponentMap().end(); it++)
			{
				auto shapePtr= it->second.lock();
				if (shapePtr == shapeComponent)
					shapeSourceIndex= listIndex;
				comboEntries.push_back({shapePtr, shapePtr->getName()});
				listIndex++;
			}
		}
		break;
		case eShapeType::model:
		{
			auto modelSystem= ownerGraph->getObjectSystemOfType<ModelShapeSystem>();
			for (auto it= modelSystem->getComponentMap().begin(); it != modelSystem->getComponentMap().end(); it++)
			{
				auto shapePtr= it->second.lock();
				if (shapePtr == shapeComponent)
					shapeSourceIndex= listIndex;
				comboEntries.push_back({shapePtr, shapePtr->getName()});
				listIndex++;
			}
		}
		break;
		default:
			break;
		}
	}

	inline int getCurrentShapeIndex() const { return shapeSourceIndex; }

	inline ShapeComponentPtr getEntryShape(int index)
	{
		assert(index >= 0 && index < (int)comboEntries.size());
		return comboEntries[index].shape;
	}

	virtual int getEntryCount() const override { return (int)comboEntries.size(); }

	virtual const std::string& getEntryDisplayString(int index) const override
	{
		assert(index >= 0 && index < (int)comboEntries.size());
		return comboEntries[index].entryString;
	}

private:
	struct ComboEntry
	{
		ShapeComponentPtr shape;
		std::string entryString;
	};

	std::vector<ComboEntry> comboEntries;
	int shapeSourceIndex= -1;
};

// -- GraphShapePropertyConfig -----
configuru::Config GraphShapePropertyConfig::writeToJSON()
{
	configuru::Config pt= GraphPropertyConfig::writeToJSON();

	pt["shape_type"]= (shapeType != eShapeType::INVALID) ? k_shapeTypeStrings[(int)shapeType]
														 : k_shapeTypeStrings[(int)eShapeType::quad];
	pt["shape_name"]= shapeName;

	return pt;
}

void GraphShapePropertyConfig::readFromJSON(const configuru::Config& pt)
{
	const std::string shapeTypeString= pt.get_or<std::string>("shape_type", k_shapeTypeStrings[(int)eShapeType::quad]);
	shapeType= StringUtils::FindEnumValue<eShapeType>(shapeTypeString, k_shapeTypeStrings);
	shapeName= pt.get_or<std::string>("shape_name", "");

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphShapeProperty -----
bool GraphShapeProperty::loadFromConfig(GraphPropertyConfigConstPtr propConfig, const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto& shapePropConfig= std::static_pointer_cast<const GraphShapePropertyConfig>(propConfig);
		if (!shapePropConfig->shapeName.empty() && shapePropConfig->shapeType != eShapeType::INVALID)
		{
			switch (shapePropConfig->shapeType)
			{
			case eShapeType::quad:
			{
				auto quadSystem= getOwnerGraph()->getObjectSystemOfType<QuadShapeSystem>();
				setShapeComponent(quadSystem->getQuadShapeByName(shapePropConfig->shapeName));
				m_shapeType= eShapeType::quad;
			}
			break;
			case eShapeType::box:
			{
				auto boxSystem= getOwnerGraph()->getObjectSystemOfType<BoxShapeSystem>();
				setShapeComponent(boxSystem->getBoxShapeByName(shapePropConfig->shapeName));
				m_shapeType= eShapeType::box;
			}
			break;
			case eShapeType::model:
			{
				auto modelSystem= getOwnerGraph()->getObjectSystemOfType<ModelShapeSystem>();
				setShapeComponent(modelSystem->getModelShapeByName(shapePropConfig->shapeName));
				m_shapeType= eShapeType::model;
			}
			break;
			default:
				MIKAN_LOG_ERROR("GraphShapeProperty::loadFromConfig")
					<< "Invalid shape name: " << shapePropConfig->shapeName;
				setShapeComponent(ShapeComponentPtr());
				m_shapeType= eShapeType::INVALID;
			}

			return m_shapeType != eShapeType::INVALID;
		}
		else
		{
			setShapeComponent(ShapeComponentPtr());
			return true;
		}
	}

	return false;
}

void GraphShapeProperty::saveToConfig(GraphPropertyConfigPtr config) const
{
	auto shapePropConfig= std::static_pointer_cast<GraphShapePropertyConfig>(config);

	if (m_shapeComponent != nullptr)
	{
		ShapeComponentDefinitionPtr definition= m_shapeComponent->getShapeComponentDefinition();

		shapePropConfig->shapeName= definition->getComponentName();
		shapePropConfig->id= definition->getComponentId();
		shapePropConfig->shapeType=
			ShapeUtils::getShapeType(getOwnerGraph()->getOwnerProject(), definition->getComponentId());
	}
	else
	{
		shapePropConfig->shapeName= "";
		shapePropConfig->id= -1;
		shapePropConfig->shapeType= eShapeType::INVALID;
	}

	GraphProperty::saveToConfig(config);
}

void GraphShapeProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto shapeNode= m_ownerGraph->createTypedNode<ShapeNode>(editorState);

	auto self= std::static_pointer_cast<GraphShapeProperty>(shared_from_this());
	shapeNode->setShapeSource(self);
}

void GraphShapeProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	MkGuiStyleConstPtr propertyStyle= editorState.styleManager->getStyle("node_editor_property_value");

	if (MkGui::drawPropertySheetHeader(editorState.styleManager->getStyle("node_editor_panel_header"),
									   locLabel("graphProperties.shapeHeader")))
	{
		// Name
		std::string name= m_shapeComponent ? m_shapeComponent->getName() : locText("graphProperties.noShape");
		MkGui::drawStaticTextProperty(propertyStyle, locText("graphProperties.name"), name);

		// Shape Type
		int shapeTypeIndex= (int)m_shapeType;
		if (ImGui::Combo(locLabel("graphProperties.type"), &shapeTypeIndex, k_szShapeTypeStrings,
						 (int)eShapeType::COUNT))
		{
			setShapeComponent(ShapeComponentPtr());
			m_shapeType= (eShapeType)shapeTypeIndex;
		}

		// Shape selection
		ShapeComboDataSource dataSource(m_ownerGraph, m_shapeComponent, m_shapeType);
		int selectedIndex= dataSource.getCurrentShapeIndex();
		if (MkGui::drawComboBoxProperty(propertyStyle, "shapeSelection", locText("graphProperties.source"), &dataSource,
										selectedIndex))
		{
			setShapeComponent(dataSource.getEntryShape(selectedIndex));
		}
	}
}

const ImVec4 GraphShapeProperty::editorGetIconColor() const { return MkGui::getComponentColor(); }
