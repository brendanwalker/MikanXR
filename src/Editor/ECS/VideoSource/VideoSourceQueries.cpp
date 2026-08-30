#include "VideoSourceQueries.h"
#include "ARKitVideoSourceSystem.h"
#include "ARKitVideoSourceComponent.h"
#include "NetworkVideoSourceSystem.h"
#include "NetworkVideoSourceComponent.h"
#include "ProjectManager.h"
#include "USBVideoSourceSystem.h"
#include "USBVideoSourceComponent.h"

namespace VideoSourceQueries
{
VideoSourceIdList getVideoSourceIdList(ProjectManagerConstPtr projectManager)
{
	VideoSourceIdList videoSourceIdList;

	auto networkVideoSourceSystem= projectManager->getSystemOfType<NetworkVideoSourceSystem>();
	auto networkVideoSourceIds= networkVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), networkVideoSourceIds.begin(), networkVideoSourceIds.end());

	auto usbVideoSourceSystem= projectManager->getSystemOfType<USBVideoSourceSystem>();
	auto usbVideoSourceIds= usbVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), usbVideoSourceIds.begin(), usbVideoSourceIds.end());

	auto arkitVideoSourceSystem= projectManager->getSystemOfType<ARKitVideoSourceSystem>();
	auto arkitVideoSourceIds= arkitVideoSourceSystem->getVideoSourceIdList();
	videoSourceIdList.insert(videoSourceIdList.end(), arkitVideoSourceIds.begin(), arkitVideoSourceIds.end());

	return videoSourceIdList;
}

VideoSourceComponentPtr getVideoSourceById(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId)
{
	auto networkVideoSourceSystem= projectManager->getSystemOfType<NetworkVideoSourceSystem>();
	auto networkVideoSourcePtr= networkVideoSourceSystem->getTypedComponentById(videoSourceId);
	if (networkVideoSourcePtr)
	{
		return networkVideoSourcePtr;
	}

	auto usbVideoSourceSystem= projectManager->getSystemOfType<USBVideoSourceSystem>();
	auto usbVideoSourcePtr= usbVideoSourceSystem->getTypedComponentById(videoSourceId);
	if (usbVideoSourcePtr)
	{
		return usbVideoSourcePtr;
	}

	auto arkitVideoSourceSystem= projectManager->getSystemOfType<ARKitVideoSourceSystem>();
	auto arkitVideoSourcePtr= arkitVideoSourceSystem->getTypedComponentById(videoSourceId);
	if (arkitVideoSourcePtr)
	{
		return arkitVideoSourcePtr;
	}

	return VideoSourceComponentPtr();
}

eVideoSourceType getVideoSourceType(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId)
{
	auto networkVideoSourceSystem= projectManager->getSystemOfType<NetworkVideoSourceSystem>();
	auto networkVideoSourcePtr= networkVideoSourceSystem->getTypedComponentById(videoSourceId);
	if (networkVideoSourcePtr)
	{
		return eVideoSourceType::networked;
	}

	auto usbVideoSourceSystem= projectManager->getSystemOfType<USBVideoSourceSystem>();
	auto usbVideoSourcePtr= usbVideoSourceSystem->getTypedComponentById(videoSourceId);
	if (usbVideoSourcePtr)
	{
		return eVideoSourceType::usb;
	}

	auto arkitVideoSourceSystem= projectManager->getSystemOfType<ARKitVideoSourceSystem>();
	auto arkitVideoSourcePtr= arkitVideoSourceSystem->getTypedComponentById(videoSourceId);
	if (arkitVideoSourcePtr)
	{
		return eVideoSourceType::arkit;
	}

	return eVideoSourceType::INVALID;
}

bool removeVideoSource(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId)
{
	switch (getVideoSourceType(projectManager, videoSourceId))
	{
	case eVideoSourceType::networked:
	{
		auto networkVideoSourceSystem= projectManager->getSystemOfType<NetworkVideoSourceSystem>();
		return networkVideoSourceSystem->removeObjectByPrimaryComponentId(videoSourceId);
	}
	case eVideoSourceType::usb:
	{
		auto usbVideoSourceSystem= projectManager->getSystemOfType<USBVideoSourceSystem>();
		return usbVideoSourceSystem->removeObjectByPrimaryComponentId(videoSourceId);
	}
	case eVideoSourceType::arkit:
	{
		auto arkitVideoSourceSystem= projectManager->getSystemOfType<ARKitVideoSourceSystem>();
		return arkitVideoSourceSystem->removeObjectByPrimaryComponentId(videoSourceId);
	}
	}

	return false;
}
} // namespace VideoSourceQueries
