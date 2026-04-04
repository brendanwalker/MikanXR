#include "AppStage.h"
#include "Shared/GuiPanel_SceneComponent.h"
#include "MikanCoreTypes.h"

#include "imgui.h"

bool GuiPanel_SceneComponent::init(AppStage* ownerAppStage)
{
	return initTypedPropertyInterface<SceneComponent>(ownerAppStage);
}

bool GuiPanel_SceneComponent::setComponent(MikanComponentPtr component)
{
	if (GuiPanel_MikanComponent::setComponent(component))
	{
		m_compositorIdList.clear();

		SceneComponentPtr sceneComp = getSceneComponent();
		if (sceneComp)
		{
			m_compositorIdList = sceneComp->getOutputCompositorIDs();
		}

		return true;
	}

	return false;
}

void GuiPanel_SceneComponent::render(float deltaSeconds)
{
	GuiPanel_MikanComponent::render(deltaSeconds);

	SceneComponentPtr sceneComp = getSceneComponent();
	if (!sceneComp)
		return;

	ImGui::Separator();

	// Compositor dropdown
	{
		int currentCompositorId = sceneComp->getSceneComponentDefinition()->getDisplayCompositorId();
		const std::string previewStr = std::to_string(currentCompositorId);

		if (ImGui::BeginCombo("Display Compositor", previewStr.c_str()))
		{
			// Refresh compositor list each time combo opens
			std::vector<int> freshList = sceneComp->getOutputCompositorIDs();
			for (int compositorId : freshList)
			{
				const bool bSelected = (compositorId == currentCompositorId);
				const std::string label = std::to_string(compositorId);

				if (ImGui::Selectable(label.c_str(), bSelected))
				{
					addUpdateCallback([sceneComp, compositorId]() {
						sceneComp->getSceneComponentDefinition()->setDisplayCompositorId(compositorId);
					});
				}
			}
			ImGui::EndCombo();
		}
	}
}

SceneComponentPtr GuiPanel_SceneComponent::getSceneComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::dynamic_pointer_cast<SceneComponent>(component);
	}
	return nullptr;
}
