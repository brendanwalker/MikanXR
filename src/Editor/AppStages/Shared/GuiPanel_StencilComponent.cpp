#include "AppStage.h"
#include "Shared/GuiPanel_StencilComponent.h"
#include "AnchorObjectSystem.h"
#include "QuadStencilComponent.h"
#include "QuadStencilSystem.h"
#include "BoxStencilComponent.h"
#include "BoxStencilSystem.h"
#include "ModelStencilComponent.h"
#include "ModelStencilSystem.h"
#include "MikanCoreTypes.h"

#include "imgui.h"

void GuiPanel_StencilComponent::onConstruct()
{
}

bool GuiPanel_StencilComponent::setComponent(MikanComponentPtr component)
{
	if (GuiPanel_MikanComponent::setComponent(component))
	{
		rebuildTransformIdList();
		return true;
	}

	return false;
}

void GuiPanel_StencilComponent::render(float deltaSeconds)
{
	// Auto-render base properties
	GuiPanel_MikanComponent::render(deltaSeconds);

	StencilComponentPtr stencilComponent = getStencilComponent();
	if (!stencilComponent)
		return;

	ImGui::Separator();

	// Parent transform selection dropdown
	// TODO: Turn this into a DataSource adapter
	{
		int currentParentId = stencilComponent->getStencilComponentDefinition()->getParentTransformId();
		const char* previewLabel = (currentParentId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentParentId).c_str();
		if (ImGui::BeginCombo("Parent", previewLabel))
		{
			for (int transformId : m_transformIdList)
			{
				const bool bSelected = (transformId == currentParentId);
				const std::string label = (transformId == INVALID_MIKAN_ID) ? "None" : std::to_string(transformId);

				if (ImGui::Selectable(label.c_str(), bSelected))
				{
					addUpdateCallback([stencilComponent, transformId]() {
						stencilComponent->getStencilComponentDefinition()->setParentTransformId(transformId);
					});
				}
			}
			ImGui::EndCombo();
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

void GuiPanel_StencilComponent::rebuildTransformIdList()
{
	// Rebuild anchor ID list
	m_transformIdList.clear();
	m_transformIdList.push_back(INVALID_MIKAN_ID); // "none" option

	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		component->getObjectSystemOfType<AnchorObjectSystem>()->getTypedComponentIdList(m_transformIdList);
		component->getObjectSystemOfType<QuadStencilSystem>()->getTypedComponentIdList(m_transformIdList);
		component->getObjectSystemOfType<BoxStencilSystem>()->getTypedComponentIdList(m_transformIdList);
		component->getObjectSystemOfType<ModelStencilSystem>()->getTypedComponentIdList(m_transformIdList);
	}
}

bool GuiPanel_QuadStencilComponent::init(AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<QuadStencilComponent>(ownerAppStage);
}

bool GuiPanel_BoxStencilComponent::init(AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<BoxStencilComponent>(ownerAppStage);
}

bool GuiPanel_ModelStencilComponent::init(AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<ModelStencilComponent>(ownerAppStage);
}
