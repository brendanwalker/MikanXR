//-- includes -----
#include "App.h"
#include "VRDeviceManager.h"
#include "VRDeviceEnumerator.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "ProfileConfig.h"
#include "SteamVRManager.h"
#include "VRDeviceView.h"

#include <easy/profiler.h>

VRDeviceManager* VRDeviceManager::m_instance= nullptr;

//-- Tracker Manager -----
VRDeviceManager::VRDeviceManager()
	: DeviceManager()
	, m_steamVRManager(new SteamVRManager(this))
{
	m_instance= this;
}

VRDeviceManager::~VRDeviceManager()
{
	delete m_steamVRManager;
	m_instance = nullptr;
}

bool VRDeviceManager::startup(class IGlWindow *ownerWindow)
{
	EASY_FUNCTION();

	bool bSuccess = DeviceManager::startup(ownerWindow);

	if (bSuccess && !m_steamVRManager->startup(ownerWindow))
	{
		MIKAN_LOG_ERROR("VRTrackerManager::init") << "Failed to initialize the SteamVR manager";
		bSuccess = false;
	}

	ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	profileConfig->OnMarkedDirty += MakeDelegate(this, &VRDeviceManager::onProfileConfigMarkedDirty);
	onVRTrackingOffsetChanged(profileConfig);

	return bSuccess;
}

void VRDeviceManager::update(float deltaTime)
{
	EASY_FUNCTION();

	m_steamVRManager->update(deltaTime);
}

void VRDeviceManager::shutdown()
{
	ProfileConfigPtr profileConfig = App::getInstance()->getProfileConfig();
	profileConfig->OnMarkedDirty -= MakeDelegate(this, &VRDeviceManager::onProfileConfigMarkedDirty);

	m_steamVRManager->shutdown();

	DeviceManager::shutdown();
}

void VRDeviceManager::onProfileConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(ProfileConfig::k_vrDevicePoseOffsetPropertyId))
	{
		onVRTrackingOffsetChanged(std::static_pointer_cast<ProfileConfig>(configPtr));
	}
}

void VRDeviceManager::onVRTrackingOffsetChanged(ProfileConfigPtr config)
{
	m_vrDevicePoseOffset= MikanMatrix4f_to_glm_mat4(config->vrDevicePoseOffset);
}

void VRDeviceManager::closeAllVRTrackers()
{
	for (int VRTrackerId = 0; VRTrackerId < k_max_devices; ++VRTrackerId)
	{
		VRDeviceViewPtr tracker_view = getVRDeviceViewById(VRTrackerId);

		if (tracker_view->getIsOpen())
		{
			tracker_view->close();
		}
	}
}

DeviceEnumerator* VRDeviceManager::allocateDeviceEnumerator()
{
	return new VRDeviceEnumerator(this);
}

void VRDeviceManager::freeDeviceEnumerator(DeviceEnumerator* enumerator)
{
	delete static_cast<VRDeviceEnumerator*>(enumerator);
}

DeviceView* VRDeviceManager::allocateDeviceView(int device_id)
{
	return new VRDeviceView(device_id);
}

VRDeviceViewPtr VRDeviceManager::getVRDeviceViewById(int device_id) const
{
	assert(m_deviceViews != nullptr);

	return std::static_pointer_cast<VRDeviceView>(m_deviceViews[device_id]);
}

VRDeviceViewPtr VRDeviceManager::getVRDeviceViewByPath(const std::string& devicePath) const
{
	for (int VRTrackerId = 0; VRTrackerId < k_max_devices; ++VRTrackerId)
	{
		VRDeviceViewPtr devicePtr = getVRDeviceViewById(VRTrackerId);

		if (devicePtr && devicePtr->getDevicePath() == devicePath)
		{
			return devicePtr;
		}
	}

	return VRDeviceViewPtr();
}

VRDeviceList VRDeviceManager::getVRDeviceList() const
{
	VRDeviceList result;

	for (int VRTrackerId = 0; VRTrackerId < k_max_devices; ++VRTrackerId)
	{
		VRDeviceViewPtr VRTrackerPtr = getVRDeviceViewById(VRTrackerId);

		if (VRTrackerPtr->getIsOpen())
		{
			result.push_back(VRTrackerPtr);
		}
	}

	return result;
}

VRDeviceList VRDeviceManager::getFilteredVRDeviceList(eDeviceType deviceType) const
{
	VRDeviceList result;

	for (int VRTrackerId = 0; VRTrackerId < k_max_devices; ++VRTrackerId)
	{
		VRDeviceViewPtr VRTrackerPtr = getVRDeviceViewById(VRTrackerId);

		if (VRTrackerPtr->getIsOpen() && VRTrackerPtr->getVRDeviceType() == deviceType)
		{
			result.push_back(VRTrackerPtr);
		}
	}

	return result;
}

void VRDeviceManager::onActiveDeviceListChanged()
{
	updateConnectedDeviceViews();

	if (OnDeviceListChanged)
	{
		OnDeviceListChanged();
	}
}

void VRDeviceManager::onDevicePropertyChanged(int deviceId)
{
	VRDeviceViewPtr tracker_view = getVRDeviceViewById(deviceId);

	if (tracker_view != nullptr && tracker_view->getIsOpen())
	{
		tracker_view->getVRDeviceInterface()->updateProperties();
	}
}

void VRDeviceManager::onDevicePosesChanged(int64_t newVRFrameIndex)
{
	for (int VRTrackerId = 0; VRTrackerId < k_max_devices; ++VRTrackerId)
	{
		VRDeviceViewPtr VRTrackerPtr = getVRDeviceViewById(VRTrackerId);

		if (VRTrackerPtr->getIsOpen())
		{
			VRTrackerPtr->getVRDeviceInterface()->updatePose();
		}
	}

	if (OnDevicePosesChanged)
	{
		OnDevicePosesChanged(newVRFrameIndex);
	}
}

// -- VRDeviceListIterator -----
VRDeviceListIterator::VRDeviceListIterator(eDeviceType deviceType)
{
	m_vrDeviceList = VRDeviceManager::getInstance()->getFilteredVRDeviceList(deviceType);
	m_listIndex = -1;

	for (int testIndex = 0; testIndex < m_vrDeviceList.size(); ++testIndex)
	{
		VRDeviceViewPtr vrDevicePtr = m_vrDeviceList[testIndex];

		if (vrDevicePtr)
		{
			m_listIndex = testIndex;
			break;
		}
	}
}

VRDeviceViewPtr VRDeviceListIterator::getCurrent() const
{
	return
		(m_listIndex >= 0 && m_listIndex < m_vrDeviceList.size())
		? m_vrDeviceList[m_listIndex]
		: VRDeviceViewPtr();
}

bool VRDeviceListIterator::goPrevious()
{
	int sourceCount = (int)m_vrDeviceList.size();
	int oldListIndex = m_listIndex;

	m_listIndex =
		(sourceCount > 0)
		? (m_listIndex + sourceCount - 1) % sourceCount
		: -1;

	return m_listIndex != oldListIndex;
}

bool VRDeviceListIterator::goNext()
{
	int sourceCount = (int)m_vrDeviceList.size();
	int oldListIndex = m_listIndex;

	m_listIndex =
		(sourceCount > 0)
		? (m_listIndex + 1) % sourceCount
		: -1;

	return m_listIndex != oldListIndex;
}