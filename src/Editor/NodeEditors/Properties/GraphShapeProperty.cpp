#include "GraphShapeProperty.h"
#include "Logger.h"
#include "NodeEditorState.h"
#include "NodeEditorUI.h"
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
class ShapeComboDataSource : public NodeEditorUI::ComboBoxDataSource
{
public:
	ShapeComboDataSource(
		NodeGraphPtr ownerGraph,
		ShapeComponentPtr shapeComponent,
		eShapeType shapeType)
	{
		if (!ownerGraph)
			return;

		int listIndex = 0;

		switch (shapeType)
		{
			case eShapeType::quad:
				{
					auto quadSystem = ownerGraph->getObjectSystemOfType<QuadShapeSystem>();
					for (auto it = quadSystem->getComponentMap().begin();
						 it != quadSystem->getComponentMap().end(); it++)
					{
						auto shapePtr = it->second.lock();
						if (shapePtr == shapeComponent)
							shapeSourceIndex = listIndex;
						comboEntries.push_back({shapePtr, shapePtr->getName()});
						listIndex++;
					}
				}
				break;
			case eShapeType::box:
				{
					auto boxSystem = ownerGraph->getObjectSystemOfType<BoxShapeSystem>();
					for (auto it = boxSystem->getComponentMap().begin();
						 it != boxSystem->getComponentMap().end(); it++)
					{
						auto shapePtr = it->second.lock();
						if (shapePtr == shapeComponent)
							shapeSourceIndex = listIndex;
						comboEntries.push_back({shapePtr, shapePtr->getName()});
						listIndex++;
					}
				}
				break;
			case eShapeType::model:
				{
					auto modelSystem = ownerGraph->getObjectSystemOfType<ModelShapeSystem>();
					for (auto it = modelSystem->getComponentMap().begin();
						 it != modelSystem->getComponentMap().end(); it++)
					{
						auto shapePtr = it->second.lock();
						if (shapePtr == shapeComponent)
							shapeSourceIndex = listIndex;
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

	virtual int getEntryCount() override { return (int)comboEntries.size(); }

	virtual const std::string& getEntryDisplayString(int index) override
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
	int shapeSourceIndex = -1;
};

// -- GraphShapePropertyConfig -----
configuru::Config GraphShapePropertyConfig::writeToJSON()
{
	configuru::Config pt = GraphPropertyConfig::writeToJSON();

	pt["shape_type"] =
		(shapeType != eShapeType::INVALID)
		? k_shapeTypeStrings[(int)shapeType]
		: k_shapeTypeStrings[(int)eShapeType::quad];
	pt["shape_name"] = shapeName;

	return pt;
}

void GraphShapePropertyConfig::readFromJSON(const configuru::Config& pt)
{
	const std::string shapeTypeString =
		pt.get_or<std::string>(
			"shape_type",
			k_shapeTypeStrings[(int)eShapeType::quad]);
	shapeType =
		StringUtils::FindEnumValue<eShapeType>(
			shapeTypeString,
			k_shapeTypeStrings);
	shapeName = pt.get_or<std::string>("shape_name", "");

	GraphPropertyConfig::readFromJSON(pt);
}

// -- GraphShapeProperty -----
bool GraphShapeProperty::loadFromConfig(
	GraphPropertyConfigConstPtr propConfig,
	const NodeGraphConfig& graphConfig)
{
	if (GraphProperty::loadFromConfig(propConfig, graphConfig))
	{
		const auto& shapePropConfig = std::static_pointer_cast<const GraphShapePropertyConfig>(propConfig);
		if (!shapePropConfig->shapeName.empty() && shapePropConfig->shapeType != eShapeType::INVALID)
		{
			switch (shapePropConfig->shapeType)
			{
			case eShapeType::quad:
				{
					auto quadSystem = getOwnerGraph()->getObjectSystemOfType<QuadShapeSystem>();
					setShapeComponent(quadSystem->getQuadShapeByName(shapePropConfig->shapeName));
					m_shapeType = eShapeType::quad;
				}
				break;
			case eShapeType::box:
				{
					auto boxSystem = getOwnerGraph()->getObjectSystemOfType<BoxShapeSystem>();
					setShapeComponent(boxSystem->getBoxShapeByName(shapePropConfig->shapeName));
					m_shapeType = eShapeType::box;
				}
				break;
			case eShapeType::model:
				{
					auto modelSystem = getOwnerGraph()->getObjectSystemOfType<ModelShapeSystem>();
					setShapeComponent(modelSystem->getModelShapeByName(shapePropConfig->shapeName));
					m_shapeType = eShapeType::model;
				}
				break;
			default:
				MIKAN_LOG_ERROR("GraphShapeProperty::loadFromConfig")
					<< "Invalid shape name: " << shapePropConfig->shapeName;
				setShapeComponent(ShapeComponentPtr());
				m_shapeType = eShapeType::INVALID;
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
	auto shapePropConfig = std::static_pointer_cast<GraphShapePropertyConfig>(config);

	if (m_shapeComponent != nullptr)
	{
		ShapeComponentDefinitionPtr definition = m_shapeComponent->getShapeComponentDefinition();

		shapePropConfig->shapeName = definition->getComponentName();
		shapePropConfig->id = definition->getComponentId();
		shapePropConfig->shapeType =
			ShapeUtils::getShapeType(
				getOwnerGraph()->getOwnerProject(),
				definition->getComponentId());
	}
	else
	{
		shapePropConfig->shapeName = "";
		shapePropConfig->id = -1;
		shapePropConfig->shapeType = eShapeType::INVALID;
	}

	GraphProperty::saveToConfig(config);
}

void GraphShapeProperty::editorHandleMainFrameDragDrop(const class NodeEditorState& editorState)
{
	auto shapeNode = m_ownerGraph->createTypedNode<ShapeNode>(editorState);

	auto self = std::static_pointer_cast<GraphShapeProperty>(shared_from_this());
	shapeNode->setShapeSource(self);
}

void GraphShapeProperty::editorRenderPropertySheet(const class NodeEditorState& editorState)
{
	if (NodeEditorUI::DrawPropertySheetHeader("Shape", editorState.styleManager))
	{
		// Name
		std::string name = m_shapeComponent ? m_shapeComponent->getName() : "<No Shape>";
		NodeEditorUI::DrawStaticTextProperty("Name", name, editorState.styleManager);

		// Shape Type
		int shapeTypeIndex = (int)m_shapeType;
		if (ImGui::Combo("Type", &shapeTypeIndex, k_szShapeTypeStrings, (int)eShapeType::COUNT))
		{
			setShapeComponent(ShapeComponentPtr());
			m_shapeType = (eShapeType)shapeTypeIndex;
		}

		// Shape selection
		ShapeComboDataSource dataSource(m_ownerGraph, m_shapeComponent, m_shapeType);
		int selectedIndex = dataSource.getCurrentShapeIndex();
		if (NodeEditorUI::DrawComboBoxProperty(
				"shapeSelection",
				"Source",
				&dataSource,
				selectedIndex,
				editorState.styleManager))
		{
			setShapeComponent(dataSource.getEntryShape(selectedIndex));
		}
	}
}

const ImVec4 GraphShapeProperty::editorGetIconColor() const
{
	return NodeEditorUI::getComponentColor();
}
