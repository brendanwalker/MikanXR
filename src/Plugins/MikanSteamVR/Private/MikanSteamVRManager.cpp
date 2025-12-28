#include "MikanSteamVRManager.h"
#include "MikanSteamVRDevice.h"
#include "IMkWindow.h"
#include "IVRDevice.h"
#include "Logger.h"
#include "PathUtils.h"
#include "SteamVRResourceManager.h"
#include "StringUtils.h"

#include "openvr.h"

#define MAX_POSE_HISTORY_FRAMES 60

struct DeviceSetPoseSample
{
	vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];
	int64_t frameCounter;
};

class DeviceSetPoseHistory
{
public:
	DeviceSetPoseHistory(size_t size)
		: m_samples(std::unique_ptr< DeviceSetPoseSample[] >(new DeviceSetPoseSample[size]))
		, m_maxSize(size)
	{}

	DeviceSetPoseSample& addSample()
	{
		m_head = (m_head + 1) % m_maxSize;

		if (m_currentSize < m_maxSize)
		{
			m_currentSize++;
		}

		return m_samples[m_head];
	}

	const DeviceSetPoseSample* findSampleOfAge(int64_t framesOld) const
	{
		if (isEmpty())
		{
			return nullptr;
		}

		const int64_t newestFrameAge = m_samples[m_head].frameCounter;
		const int64_t targetFrameAge = (newestFrameAge >= framesOld) ? newestFrameAge - framesOld : 0;

		// Walk backwards looking for an old enough sample
		size_t sampleIndex = m_head;
		for (size_t iter = 0; iter < m_currentSize; ++iter)
		{
			const DeviceSetPoseSample* sample = &m_samples[sampleIndex];

			if (sample->frameCounter <= targetFrameAge)
			{
				return sample;
			}
			else
			{
				sampleIndex = (sampleIndex - 1 + m_maxSize) % m_maxSize;
			}
		}

		return nullptr;
	}

	const DeviceSetPoseSample* getNewestSample() const
	{
		return !isEmpty() ? &m_samples[m_head] : nullptr;
	}

	const DeviceSetPoseSample* getOldestSample() const
	{
		const size_t tail = (m_head - (m_currentSize - 1) + m_maxSize) % m_maxSize;

		return !isEmpty() ? &m_samples[tail] : nullptr;
	}

	void reset()
	{
		m_head = 0;
		m_currentSize = 0;
	}

	bool isEmpty() const
	{
		return m_currentSize == 0;
	}

	bool full() const
	{
		return m_currentSize >= m_maxSize;
	}

	size_t capacity() const
	{
		return m_maxSize;
	}

	size_t size() const
	{
		return m_currentSize;
	}

private:
	std::unique_ptr< DeviceSetPoseSample[] > m_samples;
	size_t m_head = 0;
	size_t m_currentSize = 0;
	const size_t m_maxSize;
};

// -- MikanSteamVRManager ------
const float MikanSteamVRManager::k_reconnectTimeoutDuration = 1.f;
const int MikanSteamVRManager::k_maxReconnectAttempts = 5;

MikanSteamVRManager::MikanSteamVRManager()
	: m_ownerWindow(nullptr)
	, m_reconnectTimeout(0.f)
	, m_reconnectAttemptCount(0)
	, m_devicePoseHistory(std::unique_ptr<DeviceSetPoseHistory>(new DeviceSetPoseHistory(MAX_POSE_HISTORY_FRAMES)))
	, m_vrFrameCounter(0)
	, m_resourceManager(std::unique_ptr<SteamVRResourceManager>(new SteamVRResourceManager()))
{}

MikanSteamVRManager::~MikanSteamVRManager()
{
}

void MikanSteamVRManager::addListener(IVRDeviceManagerListener* eventListener)
{
	auto it = std::find(m_listeners.begin(), m_listeners.end(), eventListener);
	if (it == m_listeners.end())
	{
		m_listeners.push_back(eventListener);
	}
}

void MikanSteamVRManager::removeListener(IVRDeviceManagerListener* eventListener)
{
	auto it = std::find(m_listeners.begin(), m_listeners.end(), eventListener);
	if (it != m_listeners.end())
	{
		m_listeners.erase(it);
	}
}

bool MikanSteamVRManager::startup(IMkWindow* ownerWindow)
{
	m_ownerWindow= ownerWindow;

	m_resourceManager->init(ownerWindow);

	if (!tryConnect())
	{
		m_reconnectTimeout = k_reconnectTimeoutDuration;
	}

	return true;
}

void MikanSteamVRManager::update(float deltaTime)
{
	vr::IVRSystem* vrSystem = vr::VRSystem();
	if (vrSystem == nullptr && m_reconnectAttemptCount < k_maxReconnectAttempts)
	{
		m_reconnectTimeout -= deltaTime;

		if (m_reconnectTimeout <= 0.f)
		{
			if (tryConnect())
			{
				vrSystem = vr::VRSystem();
			}
			else
			{
				m_reconnectTimeout = k_reconnectTimeoutDuration;
			}
		}
	}

	if (vrSystem != nullptr)
	{
		vr::VREvent_t Event;
		while (vrSystem->PollNextEvent(&Event, sizeof(vr::VREvent_t)))
		{
			switch (Event.eventType)
			{
				case vr::VREvent_Quit:
					MIKAN_LOG_INFO("SteamVRManager::update") << "Quit event received from SteamVR.";
					disconnect();
					break;
					//  A tracked device was plugged in or otherwise detected by the system. 
					// There is no data, but the trackedDeviceIndex will be the index of the new device.
				case vr::VREvent_TrackedDeviceActivated:
					handleTrackedDeviceActivated(Event.trackedDeviceIndex);
					break;
					// One or more of the properties of a tracked device have changed. Data is not used for this event.
				case vr::VREvent_TrackedDeviceUpdated:
					handleTrackedDevicePropertyChanged(Event.trackedDeviceIndex);
					break;
					// A tracked device was unplugged or the system is no longer able to contact it in some other way. 
					// Data is not used for this event.
				case vr::VREvent_TrackedDeviceDeactivated:
					handleTrackedDeviceDeactivated(Event.trackedDeviceIndex);
					break;
				case vr::VREvent_ChaperoneDataHasChanged:
					break;
				case vr::VREvent_ChaperoneUniverseHasChanged:
					break;
				case vr::VREvent_ChaperoneSettingsHaveChanged:
					break;
				case vr::VREvent_TrackedDeviceRoleChanged:
					break;
			}
		}
	}

	updateDevicePoses();
}

void MikanSteamVRManager::shutdown()
{
	disconnect();
}

size_t MikanSteamVRManager::getDeviceCount() const
{
	return m_activeSteamVRDeviceList.size();
}

IVRDevice* MikanSteamVRManager::getDeviceByIndex(size_t index)
{
	return index < m_activeSteamVRDeviceList.size() ? m_activeSteamVRDeviceList[index].get() : nullptr;
}

IVRDevice* MikanSteamVRManager::getDeviceByPath(const char* devicePath)
{
	auto it= std::find_if(
		m_activeSteamVRDeviceList.begin(), m_activeSteamVRDeviceList.end(), 
		[devicePath](const IVRDevicePtr& device) {
			return strncmp(device->getDevicePath(), devicePath, 512) == 0;
		});
	if (it != m_activeSteamVRDeviceList.end())
	{
		return it->get();
	}

	return nullptr;
}

bool MikanSteamVRManager::tryConnect()
{
	m_reconnectAttemptCount++;
	MIKAN_LOG_INFO("MikanSteamVRManager::startup") << "Connect attempt #" << m_reconnectAttemptCount;

	vr::EVRInitError vrInitError = vr::VRInitError_None;
	vr::IVRSystem* vrSystem = vr::VR_Init(&vrInitError, vr::EVRApplicationType::VRApplication_Overlay);
	if (vrSystem == nullptr || vrInitError != vr::VRInitError_None)
	{
		const char* szErrorMsg = vr::VR_GetVRInitErrorAsEnglishDescription(vrInitError);
		MIKAN_LOG_ERROR("MikanSteamVRManager::startup") << "Failed to initialize SteamVR: " << szErrorMsg;
		return false;
	}

	const std::filesystem::path actionManifestPath = PathUtils::getResourceDirectory() / "input";
	const std::string actionManifestPathString= actionManifestPath.string();
	vr::EVRInputError eVRInputError = vr::VRInput()->SetActionManifestPath(actionManifestPathString.c_str());
	if (eVRInputError != vr::VRInputError_None)
	{
		MIKAN_LOG_WARNING("MikanSteamVRManager::startup") << "Failed to set action manifest path: " << eVRInputError;
	}

	MIKAN_LOG_INFO("MikanSteamVRManager::startup") << "Connected to SteamVR.";

	// Fetch the initial set of connected tracked devices we care about
	m_activeSteamVRDeviceIdSet.clear();
	m_activeSteamVRDeviceList.clear();
	addConnectedDeviceIdsOfClass(vr::TrackedDeviceClass_GenericTracker);
	addConnectedDeviceIdsOfClass(vr::TrackedDeviceClass_HMD);
	addConnectedDeviceIdsOfClass(vr::TrackedDeviceClass_Controller);

	if (m_activeSteamVRDeviceIdSet.size() > 0)
	{
		for (IVRDeviceManagerListener* listener : m_listeners)
		{
			listener->onActiveDeviceListChanged(this);
		}
	}

	return true;
}

void MikanSteamVRManager::disconnect()
{
	m_resourceManager->cleanup();
	m_activeSteamVRDeviceIdSet.clear();
	m_activeSteamVRDeviceList.clear();

	if (vr::VRSystem() != nullptr)
	{
		MIKAN_LOG_INFO("MikanSteamVRManager::startup") << "Disconnected from SteamVR.";

		vr::VR_Shutdown();
	}
}

SteamVRDeviceList MikanSteamVRManager::getActiveDevices() const
{
	SteamVRDeviceList filteredDevices;

	for (const auto& devicePtr : m_activeSteamVRDeviceList)
	{
		filteredDevices.push_back(devicePtr.get());
	}

	return filteredDevices;
}

SteamVRDeviceList MikanSteamVRManager::getActiveDevicesOfType(eVRDeviceType deviceType) const
{
	SteamVRDeviceList filteredDevices;

	for (const auto& devicePtr : m_activeSteamVRDeviceList)
	{
		if (devicePtr->getDeviceType() == deviceType)
		{
			filteredDevices.push_back(devicePtr.get());
		}
	}

	return filteredDevices;
}

const vr::TrackedDevicePose_t* MikanSteamVRManager::getDevicePose(
	vr::TrackedDeviceIndex_t steamvrDeviceId,
	int vrFrameDelay) const
{
	if (m_activeSteamVRDeviceIdSet.find(steamvrDeviceId) != m_activeSteamVRDeviceIdSet.end())
	{
		// Find the frame of the desired age
		const DeviceSetPoseSample* currentPoseSet = nullptr;
		if (!m_devicePoseHistory->isEmpty())
		{
			currentPoseSet = m_devicePoseHistory->findSampleOfAge(vrFrameDelay);
			if (currentPoseSet == nullptr)
			{
				currentPoseSet = m_devicePoseHistory->getOldestSample();
			}
		}

		if (currentPoseSet != nullptr)
		{
			return &currentPoseSet->devicePoses[steamvrDeviceId];
		}
	}

	return nullptr;
}

void MikanSteamVRManager::addConnectedDeviceIdsOfClass(int deviceClassEnumValue)
{
	auto deviceClass = static_cast<vr::ETrackedDeviceClass>(deviceClassEnumValue);
	vr::TrackedDeviceIndex_t deviceIndices[vr::k_unMaxTrackedDeviceCount];
	uint32_t deviceCount =
		vr::VRSystem()->GetSortedTrackedDeviceIndicesOfClass(
			deviceClass,
			deviceIndices,
			vr::k_unMaxTrackedDeviceCount);

	for (uint32_t index = 0; index < deviceCount; ++index)
	{
		const vr::TrackedDeviceIndex_t steamVRDeviceId= deviceIndices[index];

		m_activeSteamVRDeviceIdSet.insert(steamVRDeviceId);
		
		auto devicePtr = std::make_shared<MikanSteamVRDevice>(this, steamVRDeviceId);
		devicePtr->updateProperties();

		m_activeSteamVRDeviceList.push_back(devicePtr);
	}
}

void MikanSteamVRManager::handleTrackedDeviceActivated(vr::TrackedDeviceIndex_t steamVRDeviceId)
{
	const vr::ETrackedDeviceClass deviceClass= vr::VRSystem()->GetTrackedDeviceClass(steamVRDeviceId);

	if (deviceClass == vr::TrackedDeviceClass_HMD ||
		deviceClass == vr::TrackedDeviceClass_Controller ||
		deviceClass == vr::TrackedDeviceClass_GenericTracker)
	{
		m_activeSteamVRDeviceIdSet.insert(steamVRDeviceId);

		auto devicePtr = std::make_shared<MikanSteamVRDevice>(this, steamVRDeviceId);
		devicePtr->updateProperties();

		m_activeSteamVRDeviceList.push_back(devicePtr);

		for (IVRDeviceManagerListener* listener : m_listeners)
		{
			listener->onActiveDeviceListChanged(this);
		}
	}
}

void MikanSteamVRManager::handleTrackedDeviceDeactivated(vr::TrackedDeviceIndex_t deviceIndex)
{
	const vr::ETrackedDeviceClass deviceClass = vr::VRSystem()->GetTrackedDeviceClass(deviceIndex);

	if (deviceClass == vr::TrackedDeviceClass_HMD ||
		deviceClass == vr::TrackedDeviceClass_Controller ||
		deviceClass == vr::TrackedDeviceClass_GenericTracker)
	{
		m_activeSteamVRDeviceIdSet.erase(deviceIndex);
		auto it = std::remove_if(
			m_activeSteamVRDeviceList.begin(), m_activeSteamVRDeviceList.end(),
			[deviceIndex](const IVRDevicePtr& device) {
			return device->getDeviceIndex() == deviceIndex;
		});

		for (IVRDeviceManagerListener* listener : m_listeners)
		{
			listener->onActiveDeviceListChanged(this);
		}
	}
}

void MikanSteamVRManager::handleTrackedDevicePropertyChanged(vr::TrackedDeviceIndex_t deviceIndex)
{
	auto it = std::find_if(
		m_activeSteamVRDeviceList.begin(), m_activeSteamVRDeviceList.end(),
		[deviceIndex](const IVRDevicePtr& device) {
		return device->getDeviceIndex() == deviceIndex;
	});

	if (it != m_activeSteamVRDeviceList.end())
	{
		// Refresh the device properties
		it->get()->updateProperties();

		// Tell any listeners that the device properties changed for this device index
		for (IVRDeviceManagerListener* listener : m_listeners)
		{
			listener->onDevicePropertyChanged(this, deviceIndex);
		}
	}
}

void MikanSteamVRManager::updateDevicePoses()
{
	vr::IVRSystem* vrSystem = vr::VRSystem();

	// Fetch the latest tracking data for all devices at once
	{
		DeviceSetPoseSample& newSample = m_devicePoseHistory->addSample();
		memset(&newSample, 0, sizeof(DeviceSetPoseSample));

		newSample.frameCounter = m_vrFrameCounter;
		if (vrSystem != nullptr)
		{
			vrSystem->GetDeviceToAbsoluteTrackingPose(
				vr::TrackingUniverseStanding,
				0.0f,
				newSample.devicePoses,
				vr::k_unMaxTrackedDeviceCount);
		}
		m_vrFrameCounter++;
	}

	// Tell the VRDeviceManager that the poses changed
	for (IVRDeviceManagerListener* listener : m_listeners)
	{
		listener->onDevicePosesChanged(this, m_vrFrameCounter);
	}
}