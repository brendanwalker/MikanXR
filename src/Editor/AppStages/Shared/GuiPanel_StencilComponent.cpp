#include "AppStage.h"
#include "Shared/GuiPanel_StencilComponent.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "GuiDataSource_ComboBox.h"
#include "QuadStencilComponent.h"
#include "QuadStencilSystem.h"
#include "BoxStencilComponent.h"
#include "BoxStencilSystem.h"
#include "ModelStencilComponent.h"
#include "ModelStencilSystem.h"
#include "MikanCoreTypes.h"
#include "MkGuiDrawUtils.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "TransformComponent.h"

GuiPanel_StencilComponent::GuiPanel_StencilComponent(AppStage* ownerAppStage) 
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

void GuiPanel_StencilComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	m_entityAccessor->setPropertyRenderer(
		TransformComponentDefinition::k_parentTransformIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			StencilComponentPtr stencilComponent = getStencilComponent();
			if (!stencilComponent)
				return false;

			m_parentTransformDataSource.refreshEntries();
			if (m_parentTransformDataSource.getEntryCount() == 0)
				return false;

			const MikanTransformID parentTransformId =
				stencilComponent->getStencilComponentDefinition()->getParentTransformId();
			int selectedIndex =
				m_parentTransformDataSource.getEntryIndexByComponentId(parentTransformId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle,
				"stencilParentTransformIndex",
				"Parent",
				&m_parentTransformDataSource,
				selectedIndex))
			{
				MikanComponentPtr newParent = m_parentTransformDataSource.getEntryAtIndex(selectedIndex);
				if (newParent)
				{
					addDeferredGuiEvent([stencilComponent, newParent]() {
						stencilComponent->getStencilComponentDefinition()->setParentTransformId(
							newParent->getComponentId());
					});
				}
			}
			return true;
		});
}

StencilComponentPtr GuiPanel_StencilComponent::getStencilComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<StencilComponent>(component);
	}
	return nullptr;
}

bool GuiPanel_QuadStencilComponent::init()
{
	return initTypedPropertyInterface<QuadStencilComponent>();
}

bool GuiPanel_BoxStencilComponent::init()
{
	return initTypedPropertyInterface<BoxStencilComponent>();
}

bool GuiPanel_ModelStencilComponent::init()
{
	return initTypedPropertyInterface<ModelStencilComponent>();
}
