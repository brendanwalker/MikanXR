#include "RmlDataBinding_UsbVideoSourcesList.h"
#include "MathUtility.h"
#include "MulticastDelegate.h"
#include "USBVideoSourceSystem.h"
#include "VideoSourceSystem.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlDataBinding_UsbVideoSourcesList::~RmlDataBinding_UsbVideoSourcesList()
{
	dispose();
}

bool RmlDataBinding_UsbVideoSourcesList::init(Rml::DataModelConstructor constructor)
{
	if (!RmlDataBinding::init(constructor))
	{
		return false;
	}

	auto usbVideoSystem = VideoSourceSystem::getSystem()->getUSBVideoSourceSystem();
	usbVideoSystem->OnVideoSourceListChanged += 
		MakeDelegate(this, &RmlDataBinding_UsbVideoSourcesList::rebuildSourcesList);

	// Register Data Model Fields
	constructor.Bind("sources_list", &m_sourcePathList);

	// Fill in the data model
	rebuildSourcesList();

	return true;
}

void RmlDataBinding_UsbVideoSourcesList::dispose()
{
	// Stop listening for tracker device changes
	auto usbVideoSystem = VideoSourceSystem::getSystem()->getUSBVideoSourceSystem();
	usbVideoSystem->OnVideoSourceListChanged -= 
		MakeDelegate(this, &RmlDataBinding_UsbVideoSourcesList::rebuildSourcesList);

	RmlDataBinding::dispose();
}

void RmlDataBinding_UsbVideoSourcesList::rebuildSourcesList()
{
	USBVideoSourceSystemPtr usbVideoSystem= VideoSourceSystem::getSystem()->getUSBVideoSourceSystem();

	if (usbVideoSystem->getConnectedUSBVideoSourcePaths(m_sourcePathList))
	{
		m_modelHandle.DirtyVariable("sources_list");
	}
}