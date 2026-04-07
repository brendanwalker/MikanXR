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

#include "imgui.h"

GuiPanel_StencilComponent::GuiPanel_StencilComponent(AppStage* ownerAppStage) 
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_parentTransformDataSource(
		ownerAppStage->getProjectManager(), 
		{
			{ AnchorObjectSystem::k_objectSystemClassName, AnchorComponent::k_componentClassName },
			{ QuadStencilSystem::k_objectSystemClassName, QuadStencilComponent::k_componentClassName },
			{ BoxStencilSystem::k_objectSystemClassName, BoxStencilComponent::k_componentClassName },
			{ ModelStencilSystem::k_objectSystemClassName, ModelStencilComponent::k_componentClassName }
		})
{
}

void GuiPanel_StencilComponent::onGui()
{
	// Auto-render base properties
	GuiPanel_MikanComponent::onGui();

	StencilComponentPtr stencilComponent = getStencilComponent();
	if (!stencilComponent)
		return;

	ImGui::Separator();

	// Parent transform selection dropdown
	m_parentTransformDataSource.refreshEntries();
	if (m_parentTransformDataSource.getEntryCount() > 0)
	{
		const MikanTransformID parentTransformId = 
			stencilComponent->getStencilComponentDefinition()->getParentTransformId();
		
		int selectedIndex = m_parentTransformDataSource.getEntryIndexByComponentId(parentTransformId);
		if (MkGui::drawComboBoxProperty(
			m_defaultGuiStyle,
			"stencilParentTransformIndex",
			"Parent",
			&m_parentTransformDataSource,
			selectedIndex))
		{
			MikanComponentPtr newParent= m_parentTransformDataSource.getEntryAtIndex(selectedIndex);

			if (newParent)
			{
				addDeferredGuiEvent([stencilComponent, newParent]() {
					stencilComponent->getStencilComponentDefinition()->setParentTransformId(
						newParent->getComponentId());
					});
			}
		}
	}
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
