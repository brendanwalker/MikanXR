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
const std::string VRObjectSystemConfig::k_trackerRuntimePropertyId = "currentTrackingRuntime";
const std::string VRObjectSystemConfig::k_vrDevicePoseOffsetPropertyId = "vrDevicePoseOffset";
const std::string VRObjectSystemConfig::k_assignedStagePropertyId = "assignedStageId";
const std::string VRObjectSystemConfig::k_vrDeviceListPropertyId = "vrDeviceList";

VRObjectSystemConfig::VRObjectSystemConfig(const std::string& configName)
	: CommonConfig(configName)
{
	m_vrDevicePoseOffset = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f,
	};
}

configuru::Config VRObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	// Tracker
	pt[k_trackerRuntimePropertyId] = k_trackingRuntimeStrings[(int)m_trackingRuntime];
	writeMatrix4f(pt, "vrDevicePoseOffset", m_vrDevicePoseOffset);

	return pt;
}

void VRObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	// VR Devices
	const std::string trackingRuntimeString =
		pt.get_or<std::string>(
			k_trackerRuntimePropertyId,
			k_trackingRuntimeStrings[(int)eTrackingRuntime::SteamVR]);
	m_trackingRuntime =
		StringUtils::FindEnumValue<eTrackingRuntime>(
			trackingRuntimeString,
			k_trackingRuntimeStrings);

	readMatrix4f(pt, "vrDevicePoseOffset", m_vrDevicePoseOffset);
}

void VRObjectSystemConfig::setTrackingRuntimeType(eTrackingRuntime runtimeType)
{
	if (runtimeType != m_trackingRuntime)
	{
		m_trackingRuntime= runtimeType;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackerRuntimePropertyId));
	}
}

const std::string& VRObjectSystemConfig::getDefaultVRObjectSocketName() const
{
	static std::string kDefaultInvalidSocketName= "";
	static std::string kDefaultSteamVRSocketName= "front_rolled";

	switch (m_trackingRuntime)
	{
		case eTrackingRuntime::SteamVR:
			return kDefaultSteamVRSocketName;
		default:
			return kDefaultInvalidSocketName;
	}
}

void VRObjectSystemConfig::setAssignedStageId(MikanStageID stageId)
{
	if (stageId != m_assignedStageId)
	{
		m_assignedStageId= stageId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_assignedStagePropertyId));
	}
}

void VRObjectSystemConfig::setVRDevicePoseOffset(const MikanMatrix4f& poseOffset)
{
	m_vrDevicePoseOffset= poseOffset;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrDevicePoseOffsetPropertyId));
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
	const std::string& vrDevicePath,
	const MikanTransform& xform)
{
	VRDeviceDefinitionPtr vrDeviceDefinition = 
		std::make_shared<VRDeviceDefinition>(
			m_nextVRDeviceId, vrDevicePath, xform);
	m_nextVRDeviceId++;

	vrDeviceList.push_back(vrDeviceDefinition);
	addChildConfig(vrDeviceDefinition);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrDeviceListPropertyId));

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
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrDeviceListPropertyId));

		return true;
	}

	return false;
}

void VRObjectSystemConfig::removeAllVRDevice()
{
	if (vrDeviceList.size() > 0)
	{
		vrDeviceList.clear();
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrDeviceListPropertyId));
	}
}

// -- VRObjectSystem -----
VRObjectSystemWeakPtr VRObjectSystem::s_VRObjectSystem;

bool VRObjectSystem::init()
{
	MikanObjectSystem::init();
	s_VRObjectSystem = std::static_pointer_cast<VRObjectSystem>(shared_from_this());

	// Listen for project config changes
	ProjectConfigPtr projectConfig = App::getInstance()->getProfileConfig();
	projectConfig->OnMarkedDirty +=
		MakeDelegate(this, &VRObjectSystem::onProjectConfigMarkedDirty);
	m_projectConfigWeakPtr= projectConfig;

	VRObjectSystemConfigPtr vrSystemConfig= projectConfig->vrObjectConfig;
	vrSystemConfig->OnMarkedDirty+= 
		MakeDelegate(this, &VRObjectSystem::onVRSystemConfigMarkedDirty);

	onTrackerRuntimeTypeChaged();

	return true;
}

void VRObjectSystem::update(float deltaSeconds)
{
	if (m_vrDeviceManager != nullptr)
	{
		m_vrDeviceManager->update(deltaSeconds);
	}
}

void VRObjectSystem::dispose()
{
	disposeVRDeviceManager();

	ProjectConfigPtr projectConfigPtr = m_projectConfigWeakPtr.lock();
	projectConfigPtr->OnMarkedDirty -=
		MakeDelegate(this, &VRObjectSystem::onProjectConfigMarkedDirty);

	s_VRObjectSystem.reset();
	m_vrDeviceComponents.clear();

	MikanObjectSystem::dispose();
}

void VRObjectSystem::onProjectConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(VRObjectSystemConfig::k_trackerRuntimePropertyId))
	{
		onTrackerRuntimeTypeChaged();
	}
}

void VRObjectSystem::onTrackerRuntimeTypeChaged()
{
	eTrackingRuntime desiredRuntime= getVRSystemConfigConst()->getTrackingRuntimeType();

	disposeVRDeviceManager();
	createVRDeviceManager(desiredRuntime);
}

void VRObjectSystem::onVRSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(VRObjectSystemConfig::k_assignedStagePropertyId))
	{
		onAssignedStageChaged();
	}
}

void VRObjectSystem::onAssignedStageChaged()
{
	MikanStageID newStageId= getVRSystemConfigConst()->getAssignedStageId();

	for (auto& kvpair : m_vrDeviceComponents)
	{
		VRDeviceComponentPtr vrDeviceComponent= kvpair.second.lock();

		if (vrDeviceComponent)
		{
			vrDeviceComponent->assignToStage(newStageId);
		}
	}
}

void VRObjectSystem::createVRDeviceManager(eTrackingRuntime desiredRuntime)
{
	// Select the VR Device module to load
	std::string moduleName;
	switch (desiredRuntime)
	{
	case eTrackingRuntime::SteamVR:
		moduleName= "MikanSteamVR";
		break;
	}

	// Bail if we didn't select a valid runtime type to use
	if (moduleName.empty())
		return;

	// Attempt to load the vr device module
	m_vrDeviceModule = getMikanModuleManager()->getModule<IVRDeviceModule>(moduleName);
	if (!m_vrDeviceModule)
	{
		MIKAN_LOG_ERROR("VRObjectSystem::createVRTrackingRuntime") << "Failed to load module" << moduleName;
		return;
	}

	// Attempt to create a vr device manager
	m_vrDeviceManager= m_vrDeviceModule->createVRDeviceManager();
	if (!m_vrDeviceManager)
	{
		MIKAN_LOG_WARNING("VRObjectSystem::createVRTrackingRuntime") << "Failed to create VRDeviceManger";
		return;
	}

	// Listen for device manager changes
	m_vrDeviceManager->addListener(this);

	// Rebuild attached device list
	onActiveDeviceListChanged();
}

void VRObjectSystem::disposeVRDeviceManager()
{
	disposeAllVRObjects();

	if (m_vrDeviceManager)
	{
		m_vrDeviceManager->removeListener(this);
		m_vrDeviceManager->shutdown();
		m_vrDeviceManager= nullptr;
	}

	if (m_vrDeviceModule)
	{
		getMikanModuleManager()->disposeModule(m_vrDeviceModule);
		m_vrDeviceModule= nullptr;
	}
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

void VRObjectSystem::onActiveDeviceListChanged()
{
	assert(m_vrDeviceManager);

	std::set<MikanVRDeviceID> pendingDeleteDeviceIds;
	for (const auto& kvpair : m_vrDeviceComponents)
	{
		const MikanVRDeviceID vrDeviceId= kvpair.first;

		pendingDeleteDeviceIds.insert(vrDeviceId);
	}

	for (size_t deviceIndex = 0; deviceIndex < m_vrDeviceManager->getDeviceCount(); deviceIndex++)
	{
		IVRDevice* vrDeviceInterface= m_vrDeviceManager->getDeviceByIndex(deviceIndex);
		const std::string devicePath= vrDeviceInterface->getDevicePath();

		if (!devicePath.empty())
		{
			VRDeviceComponentPtr existingVRDeice= getVRDeviceByPath(devicePath);

			if (existingVRDeice)
			{
				const MikanVRDeviceID vrDeviceId= existingVRDeice->getVRDeviceDefinition()->getVRDeviceId();

				pendingDeleteDeviceIds.erase(vrDeviceId);
			}
			else
			{
				addNewVRDevice(vrDeviceInterface);
			}
		}
	}

	for (const MikanVRDeviceID vrDeviceId : pendingDeleteDeviceIds)
	{
		removeVRDevice(vrDeviceId);
	}
}

void VRObjectSystem::onDevicePropertyChanged(int deviceId)
{

}

void VRObjectSystem::onDevicePosesChanged(int64_t newFrameId)
{
	for (const auto& kvpair : m_vrDeviceComponents)
	{
		VRDeviceComponentPtr vrDeviceComponentPtr= kvpair.second.lock();

		if (vrDeviceComponentPtr)
		{
			vrDeviceComponentPtr->refreshDevicePose();
		}
	}
}

VRDeviceComponentPtr VRObjectSystem::addNewVRDevice(IVRDevice* vrDeviceInterface)
{
	const std::string vrDevicePath= vrDeviceInterface->getDevicePath();

	VRDevicePose pose = {};
	vrDeviceInterface->getDevicePose(pose);

	MikanTransform xform;
	xform.position= {pose.position.x, pose.position.y, pose.position.z};
	xform.rotation= {pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z};
	xform.scale= {1.f, 1.f, 1.f};

	VRObjectSystemConfigPtr vrSystemConfig = getVRSystemConfig();
	MikanVRDeviceID vrDeviceId = vrSystemConfig->addNewVRDevice(vrDevicePath, xform);
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
	getVRSystemConfig()->removeAllVRDevice();
}

const ProjectConfig* VRObjectSystem::getProjectConfig() const
{
	ProjectConfigPtr projectConfigPtr= m_projectConfigWeakPtr.lock();
	if (projectConfigPtr)
	{
		return projectConfigPtr.get();
	}

	return nullptr;
}

VRObjectSystemConfigConstPtr VRObjectSystem::getVRSystemConfigConst() const
{
	return getProjectConfig()->vrObjectConfig;
}

VRObjectSystemConfigPtr VRObjectSystem::getVRSystemConfig()
{
	return std::const_pointer_cast<VRObjectSystemConfig>(getVRSystemConfigConst());
}

// -- Utility Methods
void addAllVRDevicesToMkScene(IMkScenePtr mkScenePtr)
{
	IMkScenePtr scene= mkScenePtr;
	auto vrDeviceSystem = VRObjectSystem::getSystem();

	for (const auto& kvpair : vrDeviceSystem->getVRDeviceMap())
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