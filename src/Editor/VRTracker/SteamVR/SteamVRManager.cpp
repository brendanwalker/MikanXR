#include "App.h"
#include "SteamVRManager.h"
#include "SteamVRResourceManager.h"
#include "Logger.h"
#include "PathUtils.h"
#include "ProjectConfig.h"
#include "StringUtils.h"

#include "openvr.h"

#include <easy/profiler.h>

#define MAX_POSE_HISTORY_FRAMES 60
const float SteamVRManager::k_reconnectTimeoutDuration = 1.f;
const int SteamVRManager::k_maxReconnectAttempts = 5;

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
		for (size_t iter= 0; iter < m_currentSize; ++iter)
		{
			const DeviceSetPoseSample* sample = &m_samples[sampleIndex];

			if (sample->frameCounter <= targetFrameAge)
			{
				return sample;
			}
			else
			{
				sampleIndex= (sampleIndex - 1 + m_maxSize) % m_maxSize;
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
		const size_t tail= (m_head - (m_currentSize - 1) + m_maxSize) % m_maxSize;

		return !isEmpty() ? &m_samples[tail] : nullptr;
	}

	void reset()
	{
		m_head = 0;
		m_currentSize= 0;
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
	size_t m_head= 0;
	size_t m_currentSize= 0;
	const size_t m_maxSize;
};

SteamVRManager::SteamVRManager(IVRSystemEventListener* listener)
	: m_reconnectTimeout(0.f)
	, m_devicePoseHistory(std::unique_ptr<DeviceSetPoseHistory>(new DeviceSetPoseHistory(MAX_POSE_HISTORY_FRAMES)))
	, m_currentPoseSet(nullptr)
	, m_vrFrameCounter(0)
	, m_resourceManager(std::unique_ptr<SteamVRResourceManager>(new SteamVRResourceManager))
	, m_eventListener(listener)	
{
}

SteamVRManager::~SteamVRManager()
{
}

bool SteamVRManager::startup(class IMkWindow* ownerWindow)
{
	EASY_FUNCTION();

	m_resourceManager->init(ownerWindow);

	if (!tryConnect())
	{
		m_reconnectTimeout = k_reconnectTimeoutDuration;
	}

	return true;
}

void SteamVRManager::update(float deltaTime)
{
	EASY_FUNCTION();

	vr::IVRSystem* vrSystem= vr::VRSystem();
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
				m_reconnectTimeout= k_reconnectTimeoutDuration;
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

void SteamVRManager::shutdown()
{
	disconnect();
}

bool SteamVRManager::tryConnect()
{
	EASY_FUNCTION();

	m_reconnectAttemptCount++;
	MIKAN_LOG_INFO("SteamVRManager::startup") << "Connect attempt #" << m_reconnectAttemptCount;

	vr::EVRInitError vrInitError = vr::VRInitError_None;
	vr::IVRSystem* vrSystem = vr::VR_Init(&vrInitError, vr::EVRApplicationType::VRApplication_Overlay);
	if (vrSystem == nullptr || vrInitError != vr::VRInitError_None)
	{
		const char* szErrorMsg = vr::VR_GetVRInitErrorAsEnglishDescription(vrInitError);
		MIKAN_LOG_ERROR("SteamVRManager::startup") << "Failed to initialize SteamVR: " << szErrorMsg;
		return false;
	}

	const std::filesystem::path actionManifestPath = PathUtils::getResourceDirectory() / "input";
	const std::string actionManifestPathString= actionManifestPath.string();
	vr::EVRInputError eVRInputError = vr::VRInput()->SetActionManifestPath(actionManifestPathString.c_str());
	if (eVRInputError != vr::VRInitError_None)
	{
		MIKAN_LOG_WARNING("SteamVRManager::startup") << "Failed to set action manifest path: " << eVRInputError;
	}

	MIKAN_LOG_INFO("SteamVRManager::startup") << "Connected to SteamVR.";

	// Fetch the initial set of connected tracked devices we care about
	m_activeSteamVRDeviceIdSet.clear();
	addConnectedDeviceIdsOfClass(vr::TrackedDeviceClass_GenericTracker);
	addConnectedDeviceIdsOfClass(vr::TrackedDeviceClass_HMD);
	addConnectedDeviceIdsOfClass(vr::TrackedDeviceClass_Controller);

	if (m_activeSteamVRDeviceIdSet.size() > 0)
	{
		m_eventListener->onActiveDeviceListChanged();
	}

	return true;
}

void SteamVRManager::disconnect()
{
	EASY_FUNCTION();

	m_resourceManager->cleanup();
	m_activeSteamVRDeviceIdSet.clear();

	if (vr::VRSystem() != nullptr)
	{
		MIKAN_LOG_INFO("SteamVRManager::startup") << "Disconnected from SteamVR.";

		vr::VR_Shutdown();
	}
}

SteamVRIdList SteamVRManager::getActiveDevices() const
{
	SteamVRIdList deviceIds;

	for (SteamVRIdSetIter it = m_activeSteamVRDeviceIdSet.begin(); it != m_activeSteamVRDeviceIdSet.end(); ++it)
	{
		vr::TrackedDeviceIndex_t steamVRDeviceId = (vr::TrackedDeviceIndex_t)(*it);

		deviceIds.push_back((int)steamVRDeviceId);
	}

	return deviceIds;
}

SteamVRIdList SteamVRManager::getActiveDevicesOfType(eDeviceType deviceType) const
{
	SteamVRIdList filteredDeviceIds;
	for (SteamVRIdSetIter it = m_activeSteamVRDeviceIdSet.begin(); it != m_activeSteamVRDeviceIdSet.end(); ++it)
	{
		vr::TrackedDeviceIndex_t steamVRDeviceId= (vr::TrackedDeviceIndex_t)(*it);

		if (getDeviceType(steamVRDeviceId) == deviceType)
		{
			filteredDeviceIds.push_back((int)steamVRDeviceId);
		}
	}

	return filteredDeviceIds;
}

eDeviceType SteamVRManager::getDeviceType(vr::TrackedDeviceIndex_t steamVRDeviceId) const
{
	eDeviceType deviceType= eDeviceType::INVALID;

	if (m_activeSteamVRDeviceIdSet.find(steamVRDeviceId) != m_activeSteamVRDeviceIdSet.end())
	{
		const vr::ETrackedDeviceClass steamVRDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(steamVRDeviceId);

		switch (steamVRDeviceClass)
		{
		case vr::TrackedDeviceClass_HMD:
			deviceType = eDeviceType::HMD;
			break;
		case vr::TrackedDeviceClass_Controller:
			deviceType = eDeviceType::VRController;
			break;
		case vr::TrackedDeviceClass_GenericTracker:
			deviceType = eDeviceType::VRTracker;
			break;
		}
	}

	return deviceType;
}

const vr::TrackedDevicePose_t* SteamVRManager::getDevicePose(vr::TrackedDeviceIndex_t steamvrDeviceId) const
{
	const vr::TrackedDevicePose_t* pose= nullptr;

	if (m_currentPoseSet != nullptr &&
		m_activeSteamVRDeviceIdSet.find(steamvrDeviceId) != m_activeSteamVRDeviceIdSet.end())
	{
		pose= &m_currentPoseSet->devicePoses[steamvrDeviceId];
	}

	return pose;
}

int SteamVRManager::getDeviceVendorId(vr::TrackedDeviceIndex_t steamvrDeviceId) const
{
	int vendorId= 0;

	if (m_activeSteamVRDeviceIdSet.find(steamvrDeviceId) != m_activeSteamVRDeviceIdSet.end())
	{
		vendorId= 
			vr::VRSystem()->GetInt32TrackedDeviceProperty(
				(vr::TrackedDeviceIndex_t)steamvrDeviceId,
				vr::Prop_EdidVendorID_Int32);
	}

	return vendorId;
}

int SteamVRManager::getDeviceProductId(vr::TrackedDeviceIndex_t steamvrDeviceId) const
{
	int productId = 0;

	if (m_activeSteamVRDeviceIdSet.find(steamvrDeviceId) != m_activeSteamVRDeviceIdSet.end())
	{
		productId =
			vr::VRSystem()->GetInt32TrackedDeviceProperty(
				(vr::TrackedDeviceIndex_t)steamvrDeviceId,
				vr::Prop_EdidProductID_Int32);
	}

	return productId;
}

std::string SteamVRManager::getDevicePath(vr::TrackedDeviceIndex_t steamvrDeviceId) const
{
	vr::IVRSystem* vrSystem = vr::VRSystem();
	std::string devicePath;

	if (m_activeSteamVRDeviceIdSet.find(steamvrDeviceId) != m_activeSteamVRDeviceIdSet.end())
	{
		char serialNumber[128];
		if (vrSystem->GetStringTrackedDeviceProperty(
			(vr::TrackedDeviceIndex_t)steamvrDeviceId,
			vr::Prop_SerialNumber_String,
			serialNumber, (uint32_t)sizeof(serialNumber)) != 0)
		{
			char modelNumber[128];
			if (vrSystem->GetStringTrackedDeviceProperty(
				(vr::TrackedDeviceIndex_t)steamvrDeviceId,
				vr::Prop_ModelNumber_String,
				modelNumber, (uint32_t)sizeof(modelNumber)) != 0)
			{
				char manufacturer[128];
				if (vrSystem->GetStringTrackedDeviceProperty(
					(vr::TrackedDeviceIndex_t)steamvrDeviceId,
					vr::Prop_ManufacturerName_String,
					manufacturer, (uint32_t)sizeof(manufacturer)) != 0)
				{
					char szDevicePath[128*3];
					StringUtils::formatString(
						szDevicePath, sizeof(szDevicePath), 
						"/devices/%s/%s%s",
						manufacturer, modelNumber, serialNumber);

					devicePath= szDevicePath;
				}
			}
		}
	}

	return devicePath;
}

void SteamVRManager::addConnectedDeviceIdsOfClass(vr::ETrackedDeviceClass deviceClass)
{
	vr::TrackedDeviceIndex_t deviceIndices[vr::k_unMaxTrackedDeviceCount];
	uint32_t deviceCount =
		vr::VRSystem()->GetSortedTrackedDeviceIndicesOfClass(
			deviceClass,
			deviceIndices,
			vr::k_unMaxTrackedDeviceCount);

	for (uint32_t index = 0; index < deviceCount; ++index)
	{
		m_activeSteamVRDeviceIdSet.insert(deviceIndices[index]);
	}
}

void SteamVRManager::handleTrackedDeviceActivated(vr::TrackedDeviceIndex_t deviceIndex)
{
	const vr::ETrackedDeviceClass deviceClass= vr::VRSystem()->GetTrackedDeviceClass(deviceIndex);

	if (deviceClass == vr::TrackedDeviceClass_HMD ||
		deviceClass == vr::TrackedDeviceClass_Controller ||
		deviceClass == vr::TrackedDeviceClass_GenericTracker)
	{
		m_activeSteamVRDeviceIdSet.insert((int)deviceIndex);
		m_eventListener->onActiveDeviceListChanged();
	}
}

void SteamVRManager::handleTrackedDeviceDeactivated(vr::TrackedDeviceIndex_t deviceIndex)
{
	const vr::ETrackedDeviceClass deviceClass = vr::VRSystem()->GetTrackedDeviceClass(deviceIndex);

	if (deviceClass == vr::TrackedDeviceClass_HMD ||
		deviceClass == vr::TrackedDeviceClass_Controller ||
		deviceClass == vr::TrackedDeviceClass_GenericTracker)
	{
		m_activeSteamVRDeviceIdSet.erase((int)deviceIndex);
		m_eventListener->onActiveDeviceListChanged();
	}
}

void SteamVRManager::handleTrackedDevicePropertyChanged(vr::TrackedDeviceIndex_t deviceIndex)
{
	//TODO: need a vr::TrackedDeviceIndex_t -> deviceIndex mapping
	//m_eventListener->onDevicePropertyChanged((int)deviceIndex);
}

void SteamVRManager::updateDevicePoses()
{
	EASY_FUNCTION();

	vr::IVRSystem* vrSystem = vr::VRSystem();
	ProjectConfigPtr config= App::getInstance()->getProfileConfig();

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

	// Use the frame delay to hold on to an older frame
	m_currentPoseSet= nullptr;
	if (!m_devicePoseHistory->isEmpty())
	{
		m_currentPoseSet = m_devicePoseHistory->findSampleOfAge(config->getVRFrameDelay());
		if (!m_currentPoseSet)
		{
			m_currentPoseSet = m_devicePoseHistory->getOldestSample();
		}
	}

	// Tell the VRDeviceManager that the poses changed
	m_eventListener->onDevicePosesChanged(m_vrFrameCounter);
}