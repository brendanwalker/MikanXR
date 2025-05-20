#include "RmlDataBinding_SourcesList.h"
#include "MathUtility.h"
#include "MulticastDelegate.h"
#include "VideoSourceManager.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlDataBinding_SourcesList::~RmlDataBinding_SourcesList()
{
	dispose();
}

bool RmlDataBinding_SourcesList::init(Rml::DataModelConstructor constructor)
{
	if (!RmlDataBinding::init(constructor))
	{
		return false;
	}

	// TODO: Start listening for tracker device changes
	//auto* videoSourcesManager = VideoSourceManager::getInstance();
	//videoSourcesManager->OnDeviceListChanged += MakeDelegate(this, &RmlDataBinding_SourcesList::rebuildSourcesList);

	// Register Data Model Fields
	constructor.Bind("sources_list", &m_sourcePathList);

	// Fill in the data model
	rebuildSourcesList();

	return true;
}

void RmlDataBinding_SourcesList::dispose()
{
	// TODO: Stop listening for tracker device changes
	//SourcesManager* SourcesManager = SourcesManager::getInstance();
	//SourcesManager->OnDeviceListChanged -= MakeDelegate(this, &RmlDataBinding_SourcesList::rebuildSourcesList);

	RmlDataBinding::dispose();
}

void RmlDataBinding_SourcesList::rebuildSourcesList()
{
	VideoSourceList videoSources = VideoSourceManager::getInstance()->getVideoSourceList();

	m_sourcePathList.clear();
	for (VideoSourceViewPtr videoSourcePtr : videoSources)
	{
		m_sourcePathList.push_back(videoSourcePtr->getUSBDevicePath());
	}
	m_modelHandle.DirtyVariable("sources_list");
}