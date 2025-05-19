#include "RmlDataBinding_VRDeviceList.h"
#include "MathUtility.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

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
	VRDeviceManager* vrDeviceManager = VRDeviceManager::getInstance();
	vrDeviceManager->OnDeviceListChanged += MakeDelegate(this, &RmlDataBinding_VRDeviceList::rebuildVRTrackerList);

	// Register Data Model Fields
	constructor.Bind("vr_device_list", &m_vrDeviceList);

	// Fill in the data model
	rebuildVRDevicePaths();

	return true;
}

void RmlDataBinding_VRDeviceList::dispose()
{
	// Stop listening for tracker device changes
	VRDeviceManager* vrDeviceManager = VRDeviceManager::getInstance();
	vrDeviceManager->OnDeviceListChanged -= MakeDelegate(this, &RmlDataBinding_VRDeviceList::rebuildVRTrackerList);

	RmlDataBinding::dispose();
}

void RmlDataBinding_VRDeviceList::rebuildVRDevicePaths()
{
	VRDeviceList vrTrackers = VRDeviceManager::getInstance()->getFilteredVRDeviceList(eDeviceType::VRTracker);

	m_vrDeviceList.clear();
	for (VRDeviceViewPtr vrTrackerPtr : vrTrackers)
	{
		m_vrDeviceList.push_back(vrTrackerPtr->getDevicePath());
	}
	m_modelHandle.DirtyVariable("vr_device_list");
}