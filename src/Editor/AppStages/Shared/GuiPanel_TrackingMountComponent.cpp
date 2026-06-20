#include "AppStage.h"
#include "Shared/GuiPanel_TrackingMountComponent.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"

bool GuiPanel_TrackingMountComponent::init()
{
	m_vrObjectSystem= getOwnerAppStage()->getSystemOfType<VRObjectSystem>();
	return initTypedPropertyInterface<TrackingMountComponent>();
}

void GuiPanel_TrackingMountComponent::onConstruct()
{
	GuiPanel_MikanComponent::onConstruct();

	// VR Device path dropdown
	m_entityAccessor->setPropertyRenderer(
		TrackingMountDefinition::k_devicePathPropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			TrackingMountComponentPtr mountComp= getTrackingMountComponent();
			if (!mountComp)
				return false;

			VRObjectSystemPtr vrObjectSystem= getVRObjectSystem();
			std::vector<std::string> devicePaths;
			if (vrObjectSystem)
			{
				for (const auto& it : vrObjectSystem->getComponentMap())
				{
					auto vrDeviceComp= it.second.lock();
					if (vrDeviceComp)
					{
						devicePaths.push_back(vrDeviceComp->getVRDeviceDefinition()->getVRDevicePath());
					}
				}
			}
			m_devicePathDataSource.setEntries(devicePaths);

			const std::string& currentPath= mountComp->getTrackingMountDefinition()->getDevicePath();
			int selectedIndex= m_devicePathDataSource.getEntryIndexByString(currentPath);

			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					mountComp->makePropertyUIIdentifier(TrackingMountDefinition::k_devicePathPropertyId),
					"VR Device",
					&m_devicePathDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const std::string newPath= m_devicePathDataSource.getEntryDisplayString(selectedIndex);
					addDeferredGuiEvent([this, newPath]()
										{ setDevicePath(newPath); });
				}
			}
			return true;
		});

	// Socket name dropdown
	m_entityAccessor->setPropertyRenderer(
		TrackingMountDefinition::k_socketNamePropertyId,
		[this](const PropertyDescriptorConstPtr& /*desc*/) -> bool
		{
			TrackingMountComponentPtr mountComp= getTrackingMountComponent();
			if (!mountComp)
				return false;

			auto vrDeviceComp= getVRDeviceComponent();
			std::vector<std::string> socketNames=
				vrDeviceComp ? vrDeviceComp->getSocketNames() : std::vector<std::string>{};
			m_socketNameDataSource.setEntries(socketNames);

			const std::string& currentSocket= mountComp->getTrackingMountDefinition()->getSocketName();
			int selectedIndex= m_socketNameDataSource.getEntryIndexByString(currentSocket);

			if (MkGui::drawComboBoxProperty(
					m_defaultGuiStyle,
					mountComp->makePropertyUIIdentifier(TrackingMountDefinition::k_socketNamePropertyId),
					"Socket",
					&m_socketNameDataSource, selectedIndex))
			{
				if (selectedIndex >= 0)
				{
					const std::string newSocket= m_socketNameDataSource.getEntryDisplayString(selectedIndex);
					addDeferredGuiEvent([mountComp, newSocket]()
										{ mountComp->getTrackingMountDefinition()->setSocketName(newSocket); });
				}
			}
			return true;
		});
}

VRObjectSystemPtr GuiPanel_TrackingMountComponent::getVRObjectSystem() const
{
	return m_vrObjectSystem.lock();
}

VRDeviceComponentPtr GuiPanel_TrackingMountComponent::getVRDeviceComponent() const
{
	TrackingMountComponentPtr mountComp= getTrackingMountComponent();
	if (mountComp)
	{
		return mountComp->getVRDeviceComponent();
	}
	return nullptr;
}

TrackingMountComponentPtr GuiPanel_TrackingMountComponent::getTrackingMountComponent() const
{
	MikanComponentPtr component= m_component.lock();
	if (component)
	{
		return std::static_pointer_cast<TrackingMountComponent>(component);
	}
	return nullptr;
}

void GuiPanel_TrackingMountComponent::setDevicePath(const std::string& devicePath)
{
	TrackingMountComponentPtr mountComp= getTrackingMountComponent();
	if (mountComp)
	{
		mountComp->getTrackingMountDefinition()->setDevicePath(devicePath);
		// Clear current socket when device changes
		mountComp->getTrackingMountDefinition()->setSocketName("");
	}
}
