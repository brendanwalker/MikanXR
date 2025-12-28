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
const std::string VRObjectSystemConfig::k_nextVRDeviceIdPropertyId = "next_vr_device_id";
const std::string VRObjectSystemConfig::k_vrDeviceListPropertyId = "vrDeviceList";

VRObjectSystemConfig::VRObjectSystemConfig(const std::string& configName)
	: MikanObjectSystemDefinition(configName)
{
}

bool VRObjectSystemConfig::wantsSaveForPropertyChange(
	const ConfigPropertyChangeSet& changedPropertySet) const
{
	// We only serialized out change to the next property id
	if (changedPropertySet.hasPropertyName(k_nextVRDeviceIdPropertyId))
	{
		return true;
	}

	return false;
}

configuru::Config VRObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt[k_nextVRDeviceIdPropertyId] = m_nextVRDeviceId;

	return pt;
}

void VRObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextVRDeviceId = pt.get_or<MikanVRDeviceID>(k_nextVRDeviceIdPropertyId, m_nextVRDeviceId);
}

VRDeviceDefinitionPtr VRObjectSystemConfig::getVRDeviceConfig(MikanVRDeviceID vrDeviceId) const
{
	auto it = std::find_if(
		vrDeviceList.begin(), vrDeviceList.end(),
		[vrDeviceId](VRDeviceDefinitionPtr configPtr) {
			return configPtr->getVRDeviceId() == vrDeviceId;
		});

	if (it != vrDeviceList.end())
	{
		return VRDeviceDefinitionPtr(*it);
	}

	return VRDeviceDefinitionPtr();
}

VRDeviceDefinitionPtr VRObjectSystemConfig::getVRDeviceConfigByPath(const std::string& vrDevicePath) const
{
	auto it = std::find_if(
		vrDeviceList.begin(), vrDeviceList.end(),
		[vrDevicePath](VRDeviceDefinitionPtr configPtr) {
		return configPtr->getVRDevicePath() == vrDevicePath;
	});

	if (it != vrDeviceList.end())
	{
		return VRDeviceDefinitionPtr(*it);
	}

	return VRDeviceDefinitionPtr();
}

MikanVRDeviceID VRObjectSystemConfig::addNewVRDevice(
	eTrackingRuntime trackingRuntime,
	const std::string& vrDevicePath,
	const MikanTransform& xform)
{
	VRDeviceDefinitionPtr vrDeviceDefinition = 
		std::make_shared<VRDeviceDefinition>(
			trackingRuntime, m_nextVRDeviceId, vrDevicePath, xform);
	m_nextVRDeviceId++;

	vrDeviceList.push_back(vrDeviceDefinition);
	addChildConfig(vrDeviceDefinition);

	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_vrDeviceListPropertyId));

	return vrDeviceDefinition->getVRDeviceId();
}

bool VRObjectSystemConfig::removeVRDevice(MikanVRDeviceID vrDeviceId)
{
	auto it = std::find_if(
		vrDeviceList.begin(), vrDeviceList.end(),
		[vrDeviceId](VRDeviceDefinitionPtr configPtr) {
		return configPtr->getVRDeviceId() == vrDeviceId;
	});

	if (it != vrDeviceList.end())
	{
		removeChildConfig(*it);

		vrDeviceList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_vrDeviceListPropertyId));

		return true;
	}

	return false;
}

void VRObjectSystemConfig::removeAllVRDevice()
{
	if (vrDeviceList.size() > 0)
	{
		vrDeviceList.clear();
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_vrDeviceListPropertyId));
	}
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
bool VRObjectSystem::init()
{
	MikanObjectSystem::init();

	// Listen for project config changes
	ProjectConfigPtr projectConfig = getProjectConfig();
	projectConfig->OnPropertyChanged +=
		MakeDelegate(this, &VRObjectSystem::onProjectConfigMarkedDirty);
	m_projectConfigWeakPtr= projectConfig;

	VRObjectSystemConfigPtr vrSystemConfig= projectConfig->vrObjectConfig;
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

	// Dispose all MikanObjects first to free all allocated meshes
	MikanObjectSystem::dispose();

	// Clean out not invalidated VRDeviceComponent weak references
	m_vrDeviceComponents.clear();

	// Clean up all tracking modules
	m_trackingRuntimes.clear();
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
			IMkWindow* ownerWindow = getOwnerProjectManager()->getOwnerWindow();
			if (vrDeviceManager->startup(ownerWindow))
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
		for (const auto& kvpair : m_vrDeviceComponents)
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

void VRObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	VRDeviceComponentPtr vrDeviceComponent = objectPtr->getComponentOfType<VRDeviceComponent>();
	if (vrDeviceComponent)
	{
		removeVRDevice(vrDeviceComponent->getVRDeviceDefinition()->getVRDeviceId());
	}
}

VRDeviceComponentPtr VRObjectSystem::getVRDeviceById(MikanVRDeviceID VRId) const
{
	auto iter = m_vrDeviceComponents.find(VRId);
	if (iter != m_vrDeviceComponents.end())
	{
		return iter->second.lock();
	}

	return VRDeviceComponentPtr();
}

VRDeviceComponentPtr VRObjectSystem::getVRDeviceByPath(const std::string& VRName) const
{
	for (auto it = m_vrDeviceComponents.begin(); it != m_vrDeviceComponents.end(); it++)
	{
		VRDeviceComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getName() == VRName)
		{
			return componentPtr;
		}
	}

	return VRDeviceComponentPtr();
}

void VRObjectSystem::onActiveDeviceListChanged(IVRDeviceManager* deviceManager)
{
	eTrackingRuntime runtimeType = findTrackingRuntimeForDeviceManager(deviceManager);

	if (runtimeType != eTrackingRuntime::INVALID)
	{
		std::set<MikanVRDeviceID> pendingDeleteDeviceIds;
		for (const auto& kvpair : m_vrDeviceComponents)
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
					const MikanVRDeviceID vrDeviceId = existingVRDeice->getVRDeviceDefinition()->getVRDeviceId();

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
		for (const auto& kvpair : m_vrDeviceComponents)
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

VRDeviceComponentPtr VRObjectSystem::addNewVRDevice(
	eTrackingRuntime trackingRuntime,
	IVRDevice* vrDeviceInterface)
{
	const int vrFrameDelay = 0; // Use the latest pose for rendering
	const std::string vrDevicePath= vrDeviceInterface->getDevicePath();

	VRDevicePose pose = {};
	vrDeviceInterface->getDevicePose(vrFrameDelay, pose);

	MikanTransform xform;
	xform.position= {pose.position.x, pose.position.y, pose.position.z};
	xform.rotation= {pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z};
	xform.scale= {1.f, 1.f, 1.f};

	VRObjectSystemConfigPtr vrSystemConfig = getVRSystemConfig();
	MikanVRDeviceID vrDeviceId = vrSystemConfig->addNewVRDevice(trackingRuntime, vrDevicePath, xform);
	if (vrDeviceId != INVALID_MIKAN_ID)
	{
		VRDeviceDefinitionPtr vrDeviceDefinition = vrSystemConfig->getVRDeviceConfig(vrDeviceId);
		assert(vrDeviceDefinition != nullptr);

		return createVRObject(vrDeviceDefinition, vrDeviceInterface);
	}

	return VRDeviceComponentPtr();
}

bool VRObjectSystem::removeVRDevice(MikanVRDeviceID vrDeviceId)
{
	getVRSystemConfig()->removeVRDevice(vrDeviceId);
	disposeVRObject(vrDeviceId);

	return false;
}

VRDeviceComponentPtr VRObjectSystem::createVRObject(
	VRDeviceDefinitionPtr vrConfig,
	IVRDevice* vrDeviceInterface)
{
	VRObjectSystemConfigConstPtr vrSystemConfig = getVRSystemConfigConst();
	MikanObjectPtr vrObject = newObject();
	vrObject->setName(vrConfig->getComponentName());

	// Add spatial VR component to the object
	VRDeviceComponentPtr vrDeviceComponent = vrObject->addComponent<VRDeviceComponent>();
	vrObject->setRootComponent(vrDeviceComponent);
	vrDeviceComponent->setDefinition(vrConfig);
	vrDeviceComponent->setVRDeviceInterface(vrDeviceInterface);

	// Add render meshes
	for (size_t meshIndex = 0; meshIndex < vrDeviceInterface->getMeshCount(); meshIndex++)
	{
		IVRDeviceMesh* vrDeviceMesh= vrDeviceInterface->getMeshByIndex(meshIndex);
	}

	// Add a selection component
	SelectionComponentPtr selectionComponent=
		vrObject->addComponent<SelectionComponent>();
	selectionComponent->setIsTransformGizmoAllowed(false);

	// Init the object once all components are added
	vrObject->init();

	// Create mesh and attachment components for all the child vr object meshes
	vrDeviceComponent->rebuildSockets();
	vrDeviceComponent->rebuildMeshComponents();

	// Set initial device pose
	vrDeviceComponent->refreshDevicePose();

	// Add the VR Device component to the list of vr objects
	m_vrDeviceComponents.insert({vrConfig->getVRDeviceId(), vrDeviceComponent});

	// TODO: Attach to a Stage

	return vrDeviceComponent;
}

void VRObjectSystem::disposeVRObject(MikanVRDeviceID VRId)
{
	auto it = m_vrDeviceComponents.find(VRId);
	if (it != m_vrDeviceComponents.end())
	{
		VRDeviceComponentPtr vrDeviceComponent = it->second.lock();

		// Remove for component list
		m_vrDeviceComponents.erase(it);

		// Free the corresponding object
		deleteObject(vrDeviceComponent->getOwnerObject());
	}
}

void VRObjectSystem::disposeAllVRObjects()
{
	deleteAllObjects();
	
	auto config = getVRSystemConfig();
	if (config)
	{
		config->removeAllVRDevice();
	}
}

VRObjectSystemConfigConstPtr VRObjectSystem::getVRSystemConfigConst() const
{
	auto projectConfig= getProjectConfig();
	return projectConfig ? projectConfig->vrObjectConfig : VRObjectSystemConfigConstPtr();
}

VRObjectSystemConfigPtr VRObjectSystem::getVRSystemConfig()
{
	return std::const_pointer_cast<VRObjectSystemConfig>(getVRSystemConfigConst());
}

// -- Utility Methods
void addAllVRDevicesToMkScene(VRObjectSystemPtr vrObjectSystem, IMkScenePtr mkScenePtr)
{
	IMkScenePtr scene= mkScenePtr;

	for (const auto& kvpair : vrObjectSystem->getVRDeviceMap())
	{
		VRDeviceComponentPtr vrDeviceComponent = kvpair.second.lock();

		if (vrDeviceComponent)
		{
			vrDeviceComponent->visitAllTransformComponentsConst(
				[scene](const TransformComponent* transformComponent) {
				IMkSceneRenderableConstPtr renderable = transformComponent->getGlSceneRenderableConst();
				if (renderable)
				{
					scene->addInstance(renderable);
				}
			});
		}
	}
}