#include "AppStage.h"
#include "Shared/GuiPanel_CompositorComponent.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "MkGuiDrawUtils.h"

#include "imgui.h"

GuiPanel_CompositorComponent::GuiPanel_CompositorComponent(AppStage* ownerAppStage)
	: GuiPanel_MikanComponent(ownerAppStage)
	, m_cameraDataSource(
		ownerAppStage->getProjectManager(),
		{
			{ CameraObjectSystem::k_objectSystemClassName, CameraComponent::k_componentClassName }
		})
{
}

bool GuiPanel_CompositorComponent::init()
{
	return initTypedPropertyInterface<CompositorComponent>();
}

void GuiPanel_CompositorComponent::onConstruct()
{
	m_entityAccessor->setPropertyRenderer(
		CompositorDefinition::k_cameraIdPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			CompositorComponentPtr compositorComp = getCompositorComponent();
			if (!compositorComp)
				return false;

			m_cameraDataSource.refreshEntries();

			const MikanCameraID currentCameraId =
				compositorComp->getCompositorDefinition()->getCameraId();
			int selectedIndex =
				m_cameraDataSource.getEntryIndexByComponentId(currentCameraId);

			if (MkGui::drawComboBoxProperty(
				m_defaultGuiStyle,
				"compositorCameraIndex",
				"Camera",
				&m_cameraDataSource,
				selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					MikanComponentPtr newCamera = m_cameraDataSource.getEntryAtIndex(selectedIndex);
					if (newCamera)
					{
						addDeferredGuiEvent([compositorComp, newCamera]() {
							compositorComp->getCompositorDefinition()->setCameraId(
								(MikanCameraID)newCamera->getComponentId());
						});
					}
				}
			}
			return true;
		});
}

CompositorComponentPtr GuiPanel_CompositorComponent::getCompositorComponent() const
{
	return std::static_pointer_cast<CompositorComponent>(m_component.lock());
}
