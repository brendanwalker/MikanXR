#include "AppStage.h"
#include "Shared/GuiPanel_CompositorComponent.h"
#include "CameraObjectSystem.h"
#include "MikanCoreTypes.h"

#include "imgui.h"

bool GuiPanel_CompositorComponent::init(AppStage* ownerAppStage)
{
	m_cameraObjectSystem = ownerAppStage->getSystemOfType<CameraObjectSystem>();
	return initTypedPropertyInterface<CompositorComponent>(ownerAppStage);
}

bool GuiPanel_CompositorComponent::setComponent(MikanComponentPtr component)
{
	return GuiPanel_MikanComponent::setComponent(component);
}

void GuiPanel_CompositorComponent::render(float deltaSeconds)
{
	GuiPanel_MikanComponent::render(deltaSeconds);

	CompositorComponentPtr compositorComp = getCompositorComponent();
	if (!compositorComp)
		return;

	ImGui::Separator();

	// Camera dropdown - build list from camera object system
	{
		CameraObjectSystemPtr cameraSystem = getCameraObjectSystem();
		int currentCameraId = compositorComp->getCompositorDefinition()->getCameraId();
		const std::string previewStr = (currentCameraId == INVALID_MIKAN_ID) ? "None" : std::to_string(currentCameraId);

		if (ImGui::BeginCombo("Camera", previewStr.c_str()))
		{
			if (cameraSystem)
			{
				auto cameraSystemDef = cameraSystem->getTypedDefinition();
				if (cameraSystemDef)
				{
					for (const auto& camDef : cameraSystemDef->getAllDefinitions())
					{
						if (!camDef) continue;
						const int cameraId = (int)camDef->getComponentId();
						const bool bSelected = (cameraId == currentCameraId);

						if (ImGui::Selectable(std::to_string(cameraId).c_str(), bSelected))
						{
							addUpdateCallback([compositorComp, cameraId]() {
								compositorComp->getCompositorDefinition()->setCameraId(cameraId);
							});
						}
					}
				}
			}
			ImGui::EndCombo();
		}
	}

	ImGui::Separator();

	// Compositor graph buttons
	if (ImGui::Button("Edit Compositor Graph"))
	{
		addUpdateCallback([compositorComp]() {
			compositorComp->editCompositorGraph();
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Graph"))
	{
		addUpdateCallback([compositorComp]() {
			compositorComp->addNewCompositorGraph();
		});
	}
	ImGui::SameLine();
	if (ImGui::Button("Remove Graph"))
	{
		addUpdateCallback([compositorComp]() {
			compositorComp->removeCompositorGraph();
		});
	}
}

CameraObjectSystemPtr GuiPanel_CompositorComponent::getCameraObjectSystem() const
{
	return m_cameraObjectSystem.lock();
}

CompositorComponentPtr GuiPanel_CompositorComponent::getCompositorComponent() const
{
	return std::static_pointer_cast<CompositorComponent>(m_component.lock());
}
