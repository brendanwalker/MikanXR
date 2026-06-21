#include "AppStage.h"
#include "MarkerObjectSystem.h"
#include "MkGuiDrawUtils.h"
#include "MkGuiStyleManager.h"
#include "ProjectConfigConstants.h"
#include "Shared/GuiPanel_MarkerObjectSystem.h"

bool GuiPanel_MarkerObjectSystem::init() { return initTypedPropertyInterface<MarkerObjectSystem>(getOwnerAppStage()); }

void GuiPanel_MarkerObjectSystem::onConstruct()
{
	if (MkGuiStyleManager* mgr= getGuiStyleManager())
		m_defaultGuiStyle= mgr->getStyle("default_component_panel");

	std::vector<std::string> dictStrings;
	for (int i= 0; i < (int)eCharucoDictionaryType::COUNT; ++i)
		dictStrings.push_back(k_charucoDictionaryStrings[i]);
	m_dictDataSource.setEntries(dictStrings);

	m_entityAccessor->setPropertyRenderer(
		MarkerObjectSystemDefinition::k_arucoDictionaryTypePropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto markerSystem= getMarkerObjectSystem();
			if (!markerSystem)
				return false;
			auto def= markerSystem->getTypedDefinition();
			if (!def)
				return false;

			int selectedIndex= (int)def->getArucoDictionaryType();
			if (MkGui::drawComboBoxProperty(m_defaultGuiStyle,
											markerSystem->makePropertyUIIdentifier(
												MarkerObjectSystemDefinition::k_arucoDictionaryTypePropertyId),
											"Aruco Dictionary", &m_dictDataSource, selectedIndex))
			{
				addDeferredGuiEvent(
					[this, selectedIndex]()
					{
						if (auto d= getMarkerObjectSystemDefinition())
							d->setArucoDictionaryType((eCharucoDictionaryType)selectedIndex);
					});
			}
			return true;
		});

	m_entityAccessor->setPropertyRenderer(
		MarkerObjectSystemDefinition::k_charucoDictionaryTypePropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			auto markerSystem= getMarkerObjectSystem();
			if (!markerSystem)
				return false;
			auto def= markerSystem->getTypedDefinition();
			if (!def)
				return false;

			int selectedIndex= (int)def->getCharucoDictionaryType();
			if (MkGui::drawComboBoxProperty(m_defaultGuiStyle,
											markerSystem->makePropertyUIIdentifier(
												MarkerObjectSystemDefinition::k_charucoDictionaryTypePropertyId),
											"Charuco Dictionary", &m_dictDataSource, selectedIndex))
			{
				addDeferredGuiEvent(
					[this, selectedIndex]()
					{
						if (auto d= getMarkerObjectSystemDefinition())
							d->setCharucoDictionaryType((eCharucoDictionaryType)selectedIndex);
					});
			}
			return true;
		});
}

void GuiPanel_MarkerObjectSystem::onGui()
{
	// Auto-render all system properties (aruco/charuco dictionary, rows, cols, etc.)
	GuiPanel_MikanObjectSystem::onGui();
}

MarkerObjectSystemPtr GuiPanel_MarkerObjectSystem::getMarkerObjectSystem() const
{
	MikanObjectSystemPtr objectSystem= m_objectSystem.lock();
	if (objectSystem)
	{
		return std::static_pointer_cast<MarkerObjectSystem>(objectSystem);
	}
	return nullptr;
}

MarkerObjectSystemDefinitionPtr GuiPanel_MarkerObjectSystem::getMarkerObjectSystemDefinition() const
{
	auto markerObjectSystem= getMarkerObjectSystem();
	if (markerObjectSystem)
	{
		return markerObjectSystem->getTypedDefinition();
	}
	return nullptr;
}
