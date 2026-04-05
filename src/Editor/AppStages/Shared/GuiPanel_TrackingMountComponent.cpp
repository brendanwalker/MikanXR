#include "AppStage.h"
#include "Shared/GuiPanel_TrackingMountComponent.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"

#include "imgui.h"

bool GuiPanel_TrackingMountComponent::init()
{
	m_vrObjectSystem = getOwnerAppStage()->getSystemOfType<VRObjectSystem>();
	return initTypedPropertyInterface<TrackingMountComponent>();
}

bool GuiPanel_TrackingMountComponent::setComponent(MikanComponentPtr component)
{
	if (GuiPanel_MikanComponent::setComponent(component))
	{
		// Rebuild VR device path list
		m_vrDevicePaths.clear();
		VRObjectSystemPtr vrObjectSystem = getVRObjectSystem();
		if (vrObjectSystem)
		{
			for (const auto& it : vrObjectSystem->getComponentMap())
			{
				auto vrDeviceComp = it.second.lock();
				if (vrDeviceComp)
				{
					m_vrDevicePaths.push_back(vrDeviceComp->getVRDeviceDefinition()->getVRDevicePath());
				}
			}
		}

		// Rebuild socket names for current device
		m_socketNames.clear();
		auto vrDeviceComp = getVRDeviceComponent();
		if (vrDeviceComp)
		{
			m_socketNames = vrDeviceComp->getSocketNames();
		}

		return true;
	}

	return false;
}

void GuiPanel_TrackingMountComponent::onGui()
{
	GuiPanel_MikanComponent::onGui();

	TrackingMountComponentPtr mountComp = getTrackingMountComponent();
	if (!mountComp)
		return;

	ImGui::Separator();

	// VR device path dropdown
	{
		const std::string& currentDevicePath = mountComp->getTrackingMountDefinition()->getDevicePath();
		const char* previewLabel = currentDevicePath.empty() ? "None" : currentDevicePath.c_str();

		// Refresh VR device path list from system
		VRObjectSystemPtr vrObjectSystem = getVRObjectSystem();
		std::vector<std::string> devicePaths;
		if (vrObjectSystem)
		{
			for (const auto& it : vrObjectSystem->getComponentMap())
			{
				auto vrDeviceComp = it.second.lock();
				if (vrDeviceComp)
				{
					devicePaths.push_back(vrDeviceComp->getVRDeviceDefinition()->getVRDevicePath());
				}
			}
		}

		if (ImGui::BeginCombo("VR Device", previewLabel))
		{
			for (const std::string& devicePath : devicePaths)
			{
				const bool bSelected = (devicePath == currentDevicePath);
				if (ImGui::Selectable(devicePath.c_str(), bSelected))
				{
					addDeferredGuiEvent([this, mountComp, devicePath]() {
						setDevicePath(devicePath);
					});
				}
			}
			ImGui::EndCombo();
		}
	}

	// Socket name dropdown (for currently selected VR device)
	{
		auto vrDeviceComp = getVRDeviceComponent();
		std::vector<std::string> socketNames = vrDeviceComp ? vrDeviceComp->getSocketNames() : std::vector<std::string>{};

		const std::string& currentSocketName = mountComp->getTrackingMountDefinition()->getSocketName();
		const char* previewLabel = currentSocketName.empty() ? "None" : currentSocketName.c_str();

		if (ImGui::BeginCombo("Socket", previewLabel))
		{
			for (const std::string& socketName : socketNames)
			{
				const bool bSelected = (socketName == currentSocketName);
				if (ImGui::Selectable(socketName.c_str(), bSelected))
				{
					addDeferredGuiEvent([mountComp, socketName]() {
						mountComp->getTrackingMountDefinition()->setSocketName(socketName);
					});
				}
			}
			ImGui::EndCombo();
		}
	}
}

VRObjectSystemPtr GuiPanel_TrackingMountComponent::getVRObjectSystem() const
{
	return m_vrObjectSystem.lock();
}

VRDeviceComponentPtr GuiPanel_TrackingMountComponent::getVRDeviceComponent() const
{
	TrackingMountComponentPtr mountComp = getTrackingMountComponent();
	if (mountComp)
	{
		return mountComp->getVRDeviceComponent();
	}
	return nullptr;
}

TrackingMountComponentPtr GuiPanel_TrackingMountComponent::getTrackingMountComponent() const
{
	MikanComponentPtr component = m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<TrackingMountComponent>(component);
	}
	return nullptr;
}

void GuiPanel_TrackingMountComponent::setDevicePath(const std::string& devicePath)
{
	TrackingMountComponentPtr mountComp = getTrackingMountComponent();
	if (mountComp)
	{
		mountComp->getTrackingMountDefinition()->setDevicePath(devicePath);
		// Clear current socket when device changes
		mountComp->getTrackingMountDefinition()->setSocketName("");
	}
}
