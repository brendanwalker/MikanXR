#include "AppStage.h"
#include "Shared/GuiPanel_AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "BoxStencilSystem.h"
#include "ModelStencilComponent.h"
#include "ModelStencilSystem.h"
#include "MkGuiDrawUtils.h"
#include "QuadStencilComponent.h"
#include "QuadStencilSystem.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "TransformComponent.h"

GuiPanel_AnchorComponent::GuiPanel_AnchorComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_parentTransformDataSource(
		ownerAppStage->getProjectManager(),
		{
			{ AnchorObjectSystem::k_objectSystemClassName, AnchorComponent::k_componentClassName },
			{ SceneObjectSystem::k_objectSystemClassName, SceneComponent::k_componentClassName },
			{ QuadStencilSystem::k_objectSystemClassName, QuadStencilComponent::k_componentClassName },
			{ BoxStencilSystem::k_objectSystemClassName, BoxStencilComponent::k_componentClassName },
			{ ModelStencilSystem::k_objectSystemClassName, ModelStencilComponent::k_componentClassName }
		})
{
}

bool GuiPanel_AnchorComponent::init()
{
	return initTypedPropertyInterface<AnchorComponent>();
}

void GuiPanel_AnchorComponent::onConstruct()
{
	m_entityAccessor->setPropertyRenderer(
		TransformComponentDefinition::k_parentTransformIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			AnchorComponentPtr anchorComp =
				std::static_pointer_cast<AnchorComponent>(getComponent());
			if (!anchorComp)
				return false;

			m_parentTransformDataSource.refreshEntries();
			if (m_parentTransformDataSource.getEntryCount() == 0)
				return false;

			const MikanTransformID parentTransformId = anchorComp->getParentTransformId();
			int selectedIndex =
				m_parentTransformDataSource.getEntryIndexByComponentId(parentTransformId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle,
				anchorComp->makePropertyUIIdentifier(TransformComponentDefinition::k_parentTransformIdPropertyId),
				"Parent",
				&m_parentTransformDataSource,
				selectedIndex))
			{
				MikanComponentPtr newParent = m_parentTransformDataSource.getEntryAtIndex(selectedIndex);
				if (newParent)
				{
					addDeferredGuiEvent([anchorComp, newParent]() {
						anchorComp->getAnchorDefinition()->setParentTransformId(
							newParent->getComponentId());
					});
				}
			}
			return true;
		});
}
