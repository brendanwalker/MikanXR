#include "App.h"
#include "BoxColliderComponent.h"
#include "IMkScene.h"
#include "IVRDeviceModule.h"
#include "IVRDevice.h"
#include "TransformComponent.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanModuleManager.h"
#include "MulticastDelegate.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"

// -- VRObjectSystemConfig -----
VRObjectSystemDefinition::VRObjectSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- VRTrackingRuntime -----
class VRTrackingRuntime
{
public:
	VRTrackingRuntime() = default;
	VRTrackingRuntime(
		VRObjectSystem* ownerSystem,
		eTrackingRuntime inRuntime, 
		IVRDeviceModule* inModule, 
		IVRDeviceManagerPtr inManager)
		: m_ownerSystem(ownerSystem)
		, m_runtime(inRuntime)
		, m_vrDeviceModule(inModule)
		, m_vrDeviceManager(inManager)
	{
		if (m_vrDeviceManager)
		{
			m_vrDeviceManager->addListener(m_ownerSystem);
		}
	}

	virtual ~VRTrackingRuntime()
	{
		if (m_vrDeviceManager)
		{
			m_vrDeviceManager->removeListener(m_ownerSystem);
			m_vrDeviceManager->shutdown();
			m_vrDeviceManager = nullptr;
		}

		if (m_vrDeviceModule)
		{
			getMikanModuleManager()->disposeModule(m_vrDeviceModule);
		}
	}

	eTrackingRuntime getRuntimeType() const { return m_runtime; }
	IVRDeviceModule* getVRDeviceModule() const { return m_vrDeviceModule; }
	IVRDeviceManagerPtr getVRDeviceManager() const { return m_vrDeviceManager; }

	void update(float deltaSeconds)
	{
		if (m_vrDeviceManager)
		{
			m_vrDeviceManager->update(deltaSeconds);
		}
	}

private:
	VRObjectSystem* m_ownerSystem = nullptr;
	eTrackingRuntime m_runtime = eTrackingRuntime::INVALID;
	IVRDeviceModule* m_vrDeviceModule = nullptr;
	IVRDeviceManagerPtr m_vrDeviceManager = nullptr;
};

// -- VRObjectSystem -----
bool VRObjectSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	if (!Super::init(definitionPtr))
		return false;

	// Listen for project config changes
	ProjectConfigPtr projectConfig = getProjectConfig();
	projectConfig->OnPropertyChanged +=
		MakeDelegate(this, &VRObjectSystem::onProjectConfigMarkedDirty);
	m_projectConfigWeakPtr= projectConfig;

	VRObjectSystemDefinitionPtr vrSystemConfig= projectConfig->vrObjectConfig;
	vrSystemConfig->OnPropertyChanged+=
		MakeDelegate(this, &VRObjectSystem::onVRSystemConfigMarkedDirty);

	return true;
}

void VRObjectSystem::update(float deltaSeconds)
{
	for (auto it = m_trackingRuntimes.begin(); it != m_trackingRuntimes.end(); it++)
	{
		it->second->update(deltaSeconds);
	}
}

void VRObjectSystem::dispose()
{
	ProjectConfigPtr projectConfigPtr = m_projectConfigWeakPtr.lock();
	if (projectConfigPtr)
	{
		projectConfigPtr->OnPropertyChanged -=
			MakeDelegate(this, &VRObjectSystem::onProjectConfigMarkedDirty);
	}

	// Clean up all tracking modules
	m_trackingRuntimes.clear();

	// Dispose all MikanObjects (will clear component map)
	Super::dispose();
}

void VRObjectSystem::onProjectConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
}

void VRObjectSystem::onVRSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
}

bool VRObjectSystem::createTrackingRuntime(eTrackingRuntime desiredRuntime)
{
	if (desiredRuntime == eTrackingRuntime::INVALID)
	{
		MIKAN_LOG_WARNING("VRObjectSystem::createVRTrackingRuntime") << "Invalid tracking runtime type requested";
		return false;
	}

	const std::string runtimeName = k_trackingRuntimeStrings[(int)desiredRuntime];
	if (m_trackingRuntimes.find(desiredRuntime) != m_trackingRuntimes.end())
	{
		MIKAN_LOG_INFO("VRObjectSystem::createVRTrackingRuntime") 
			<< "Tracking runtime already exists for " << runtimeName;
		return true;
	}

	IVRDeviceModule* vrDeviceModule = nullptr;
	IVRDeviceManagerPtr vrDeviceManager;
	bool bSuccess = false;

	// Attempt to load the vr device module
	std::string moduleName = StringUtils::stringify("Mikan", runtimeName);
	vrDeviceModule = getMikanModuleManager()->getModule<IVRDeviceModule>(moduleName);
	if (vrDeviceModule)
	{
		MIKAN_LOG_INFO("VRObjectSystem::createVRTrackingRuntime")
			<< "Loaded module " << moduleName;

		// Attempt to create a vr device manager
		vrDeviceManager = vrDeviceModule->createTrackingRuntime();
		if (vrDeviceManager)
		{
			MIKAN_LOG_INFO("VRObjectSystem::createVRTrackingRuntime") 
				<< "Allocated TrackingRuntime for " << moduleName;

			// Attempt to startup the vr device manager
			IEditorWindow* ownerWindow = getOwnerProjectManager()->getOwnerWindow();
			if (vrDeviceManager->startup(ownerWindow->getGraphicsContext().get()))
			{
				MIKAN_LOG_INFO("VRObjectSystem::createVRTrackingRuntime")
					<< "Started VRDeviceManger for " << moduleName;

				// Add the new tracking runtime to the tracking system map
				m_trackingRuntimes[desiredRuntime] =
					std::make_unique<VRTrackingRuntime>(
						this, desiredRuntime, vrDeviceModule, vrDeviceManager);

				// Rebuild attached device list
				onActiveDeviceListChanged(vrDeviceManager.get());

				bSuccess = true;
			}
			else
			{
				MIKAN_LOG_WARNING("VRObjectSystem::createVRTrackingRuntime")
					<< "Failed to startup VRDeviceManger for " << moduleName;
			}
		}
		else
		{
			MIKAN_LOG_WARNING("VRObjectSystem::createVRTrackingRuntime")
				<< "Failed to allocate TrackingRuntime for " << moduleName;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("VRObjectSystem::createVRTrackingRuntime") 
			<< "Failed to load module" << moduleName;
	}

	// Clean up if anything failed
	if (!bSuccess)
	{
		if (vrDeviceManager)
		{
			vrDeviceManager->shutdown();
			vrDeviceManager = nullptr;
		}

		if (vrDeviceModule)
		{
			getMikanModuleManager()->disposeModule(vrDeviceModule);
			vrDeviceModule = nullptr;
		}		
	}

	return bSuccess;
}

eTrackingRuntime VRObjectSystem::findTrackingRuntimeForDeviceManager(
	const IVRDeviceManager* deviceManager) const
{
	for (const auto& kvpair : m_trackingRuntimes)
	{
		if (kvpair.second->getVRDeviceManager().get() == deviceManager)
		{
			return kvpair.first;
		}
	}

	return eTrackingRuntime::INVALID;
}

bool VRObjectSystem::findMikanDeviceIdForDeviceIndex(
	const IVRDeviceManager* deviceManager,
	const int deviceIndex,
	eTrackingRuntime& outRuntime,
	MikanVRDeviceID& outMikanDeviceId) const
{
	outRuntime = findTrackingRuntimeForDeviceManager(deviceManager);
	outMikanDeviceId = INVALID_MIKAN_ID;

	if (outRuntime != eTrackingRuntime::INVALID)
	{
		for (const auto& kvpair : Super::getComponentMap())
		{
			MikanVRDeviceID vrDeviceId = kvpair.first;
			VRDeviceComponentPtr vrDeviceComponentPtr = kvpair.second.lock();

			if (vrDeviceComponentPtr)
			{
				VRDeviceDefinitionPtr vrDeviceDefinition = vrDeviceComponentPtr->getVRDeviceDefinition();
				if (vrDeviceDefinition->getTrackingRuntimeType() == outRuntime)
				{
					IVRDevice* vrDeviceInterface = vrDeviceComponentPtr->getVRDeviceInterface();
					if (vrDeviceInterface && vrDeviceInterface->getDeviceIndex() == deviceIndex)
					{
						outMikanDeviceId = vrDeviceId;
						return true;
					}
				}
			}
		}
	}

	return false;
}

VRDeviceComponentPtr VRObjectSystem::getVRDeviceByPath(const std::string& VRDevicePath) const
{
	return Super::getTypedComponentByPredicate([VRDevicePath](VRDeviceComponentConstPtr componentPtr) {
		return componentPtr->getVRDeviceDefinition()->getVRDevicePath() == VRDevicePath;
	});
}

void VRObjectSystem::getVRDevicePathList(std::vector<std::string>& outDevicePathList) const
{
	outDevicePathList.clear();

	for (const auto& kvpair : Super::getComponentMap())
	{
		VRDeviceComponentPtr vrDeviceComponentPtr = kvpair.second.lock();
		if (vrDeviceComponentPtr)
		{
			VRDeviceDefinitionPtr vrDeviceDefinition = vrDeviceComponentPtr->getVRDeviceDefinition();
			const std::string devicePath = vrDeviceDefinition->getVRDevicePath();

			outDevicePathList.push_back(devicePath);
		}
	}
}

void VRObjectSystem::onActiveDeviceListChanged(IVRDeviceManager* deviceManager)
{
	eTrackingRuntime runtimeType = findTrackingRuntimeForDeviceManager(deviceManager);

	if (runtimeType != eTrackingRuntime::INVALID)
	{
		std::set<MikanVRDeviceID> pendingDeleteDeviceIds;
		for (const auto& kvpair : Super::getComponentMap())
		{
			const MikanVRDeviceID vrDeviceId = kvpair.first;
			VRDeviceComponentPtr vrDeviceComponentPtr = kvpair.second.lock();

			if (!vrDeviceComponentPtr ||
				vrDeviceComponentPtr->getVRDeviceDefinition()->getTrackingRuntimeType() == runtimeType)
			{
				pendingDeleteDeviceIds.insert(vrDeviceId);
			}
		}

		for (size_t deviceIndex = 0; deviceIndex < deviceManager->getDeviceCount(); deviceIndex++)
		{
			IVRDevice* vrDeviceInterface = deviceManager->getDeviceByIndex(deviceIndex);
			const std::string devicePath = vrDeviceInterface->getDevicePath();

			if (!devicePath.empty())
			{
				VRDeviceComponentPtr existingVRDeice = getVRDeviceByPath(devicePath);

				if (existingVRDeice)
				{
					const MikanVRDeviceID vrDeviceId = existingVRDeice->getVRDeviceDefinition()->getComponentId();

					pendingDeleteDeviceIds.erase(vrDeviceId);
				}
				else
				{
					addNewVRDevice(runtimeType, vrDeviceInterface);
				}
			}
		}

		for (const MikanVRDeviceID vrDeviceId : pendingDeleteDeviceIds)
		{
			removeVRDevice(vrDeviceId);
		}

		if (OnActiveDeviceListChanged)
		{
			OnActiveDeviceListChanged(runtimeType);
		}
	}
}

void VRObjectSystem::onDevicePropertyChanged(IVRDeviceManager* deviceManager, int deviceId)
{
	if (OnDevicePropertyChanged)
	{
		eTrackingRuntime runtimeType;
		MikanVRDeviceID vrDeviceId;

		if (findMikanDeviceIdForDeviceIndex(
				deviceManager, deviceId, 
				runtimeType, vrDeviceId))
		{
			OnDevicePropertyChanged(runtimeType, vrDeviceId);
		}
	}
}

void VRObjectSystem::onDevicePosesChanged(IVRDeviceManager* deviceManager, int64_t newFrameId)
{
	eTrackingRuntime runtimeType = findTrackingRuntimeForDeviceManager(deviceManager);

	if (runtimeType != eTrackingRuntime::INVALID)
	{
		for (const auto& kvpair : Super::getComponentMap())
		{
			VRDeviceComponentPtr vrDeviceComponentPtr = kvpair.second.lock();

			if (vrDeviceComponentPtr &&
				vrDeviceComponentPtr->getVRDeviceDefinition()->getTrackingRuntimeType() == runtimeType)
			{
				vrDeviceComponentPtr->refreshDevicePose();
			}
		}

		if (OnDevicePosesChanged)
		{
			OnDevicePosesChanged(runtimeType, newFrameId);
		}
	}
}

// -- IEntityAccessor ----
rfk::Struct const* VRObjectSystem::getClientAPIValuesStructType() const
{
	return &MikanVRObjectSystemValues::staticGetArchetype();
}


// -- IPropertyInterface ----
const std::string VRObjectSystem::k_vrDevicePathListPropertyId = "vr_device_path_list";
void VRObjectSystem::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	Super::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			VRObjectSystem::k_vrDevicePathListPropertyId, MikanVariantType::STRING_ARRAY)
		->setReadOnly());
}

bool VRObjectSystem::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	if (propertyName == k_vrDevicePathListPropertyId)
	{
		std::vector<std::string> devicePathList;
		getVRDevicePathList(devicePathList);
		outValue = devicePathList;
		return true;
	}

	return false;
}

VRDeviceComponentPtr VRObjectSystem::addNewVRDevice(
	eTrackingRuntime trackingRuntime,
	IVRDevice* vrDeviceInterface)
{
	VRDeviceComponentPtr vrDeviceComponent = 
		Super::addNewObjectByTypedDefinition(
			[trackingRuntime, vrDeviceInterface](VRDeviceDefinitionPtr def) {
				const eVRDeviceType deviceType = vrDeviceInterface->getDeviceType();
				const std::string vrDevicePath = vrDeviceInterface->getDevicePath();
				const int vrFrameDelay = 0; // Use the latest pose for rendering

				VRDevicePose pose = {};
				vrDeviceInterface->getDevicePose(vrFrameDelay, pose);
				GlmTransform glmTransform = VRDevicePose_to_GlmTransform(pose);

				// We suppress transform change auto-notifications on VRDevices
				// because they are super spammy and change every frame
				def->setDisableAutoNotifyTransformPropertyChanges(true);

				def->setTrackingRuntimeType(trackingRuntime);
				def->setVRDeviceIndex(vrDeviceInterface->getDeviceIndex());
				def->setVRDevicePath(vrDevicePath);
				def->setRelativeTransform(glmTransform);

				return true;
 			});

	if (vrDeviceComponent)
	{
		// This will dynamically attach socket and mesh components based on the device interface
		vrDeviceComponent->setVRDeviceInterface(vrDeviceInterface);
	}

	return vrDeviceComponent;
}

bool VRObjectSystem::removeVRDevice(MikanVRDeviceID vrDeviceId)
{
	return Super::removeObjectByPrimaryComponentId(vrDeviceId);
}

// -- Utility Methods
void addAllVRDevicesToMkScene(VRObjectSystemPtr vrObjectSystem, IMkScenePtr mkScenePtr)
{
	IMkScenePtr scene = mkScenePtr;

	vrObjectSystem->visitComponents([scene](VRDeviceComponentPtr vrDeviceComponent) {
		vrDeviceComponent->visitAllTransformComponentsConst(
			[scene](const TransformComponent* transformComponent) {
			IMkSceneRenderableConstPtr renderable = transformComponent->getGlSceneRenderableConst();
			if (renderable)
			{
				scene->addInstance(renderable);
			}
		});
	});
}