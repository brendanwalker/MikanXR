#include "RmlDataBinding_VRDeviceList.h"
#include "MathUtility.h"
#include "MulticastDelegate.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlDataBinding_VRDeviceList::~RmlDataBinding_VRDeviceList()
{
	dispose();
}

bool RmlDataBinding_VRDeviceList::init(Rml::DataModelConstructor constructor)
{
	if (!RmlDataBinding::init(constructor))
	{
		return false;
	}

	// Start listening for tracker device changes
	auto vrSystemConfig = VRObjectSystem::getSystem()->getVRSystemConfig();
	vrSystemConfig->OnMarkedDirty += MakeDelegate(this, &RmlDataBinding_VRDeviceList::onVRSystemConfigMarkedDirty);

	// Register Data Model Fields
	constructor.Bind("vr_device_list", &m_vrDeviceList);

	// Fill in the data model
	rebuildVRDeviceList();

	return true;
}

void RmlDataBinding_VRDeviceList::dispose()
{
	// Stop listening for tracker device changes
	auto vrSystemConfig = VRObjectSystem::getSystem()->getVRSystemConfig();
	vrSystemConfig->OnMarkedDirty += MakeDelegate(this, &RmlDataBinding_VRDeviceList::onVRSystemConfigMarkedDirty);

	RmlDataBinding::dispose();
}

void RmlDataBinding_VRDeviceList::onVRSystemConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(VRObjectSystemConfig::k_vrDeviceListPropertyId))
	{
		rebuildVRDeviceList();
	}
}

void RmlDataBinding_VRDeviceList::rebuildVRDeviceList()
{
	auto vrObjectSystem = VRObjectSystem::getSystem();

	m_vrDeviceList.clear();
	for (const auto& kvpair : vrObjectSystem->getVRDeviceMap())
	{
		VRDeviceComponentPtr vrTrackerPtr= kvpair.second.lock();

		if (vrTrackerPtr)
		{
			m_vrDeviceList.push_back(vrTrackerPtr->getVRDeviceDefinition()->getVRDevicePath());
		}
	}
	m_modelHandle.DirtyVariable("vr_device_list");
}