#include "AppStage.h"
#include "Shared/GuiPanel_SceneComponent.h"
#include "CompositorComponent.h"
#include "CompositorObjectSystem.h"
#include "LocText.h"
#include "MkGuiDrawUtils.h"

GuiPanel_SceneComponent::GuiPanel_SceneComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_compositorDataSource(ownerAppStage->getProjectManager(), {{CompositorObjectSystem::k_objectSystemClassName,
																   CompositorComponent::k_componentClassName}})
{
}

bool GuiPanel_SceneComponent::init() { return initTypedPropertyInterface<SceneComponent>(); }

void GuiPanel_SceneComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	m_entityAccessor->setPropertyRenderer(
		SceneComponentDefinition::k_displayCompositorIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			SceneComponentPtr sceneComp= getSceneComponent();
			if (!sceneComp)
				return false;

			m_compositorDataSource.refreshEntries();
			if (m_compositorDataSource.getEntryCount() == 0)
				return false;

			const MikanCompositorID currentCompositorId=
				sceneComp->getSceneComponentDefinition()->getDisplayCompositorId();
			int selectedIndex= m_compositorDataSource.getEntryIndexByComponentId(currentCompositorId);

			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					sceneComp->makePropertyUIIdentifier(SceneComponentDefinition::k_displayCompositorIdPropertyId),
					locText("componentPanel.displayCompositor"), &m_compositorDataSource, selectedIndex))
			{
				MikanComponentPtr newCompositor= m_compositorDataSource.getEntryAtIndex(selectedIndex);
				if (newCompositor)
				{
					addDeferredGuiEvent(
						[sceneComp, newCompositor]()
						{
							sceneComp->getSceneComponentDefinition()->setDisplayCompositorId(
								newCompositor->getComponentId());
						});
				}
			}
			return true;
		});
}

SceneComponentPtr GuiPanel_SceneComponent::getSceneComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::dynamic_pointer_cast<SceneComponent>(component);
	}
	return nullptr;
}
