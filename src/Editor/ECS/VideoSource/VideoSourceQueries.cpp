#include "VideoSourceQueries.h"
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

		auto networkVideoSourceSystem = projectManager->getSystemOfType<NetworkVideoSourceSystem>();
		auto networkVideoSourceIds = networkVideoSourceSystem->getVideoSourceIdList();
		videoSourceIdList.insert(videoSourceIdList.end(), networkVideoSourceIds.begin(), networkVideoSourceIds.end());

		auto usbVideoSourceSystem = projectManager->getSystemOfType<USBVideoSourceSystem>();
		auto usbVideoSourceIds = usbVideoSourceSystem->getVideoSourceIdList();
		videoSourceIdList.insert(videoSourceIdList.end(), usbVideoSourceIds.begin(), usbVideoSourceIds.end());

		return videoSourceIdList;
	}

	VideoSourceComponentPtr getVideoSourceById(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId)
	{
		auto networkVideoSourceSystem = projectManager->getSystemOfType<NetworkVideoSourceSystem>();
		auto networkVideoSourcePtr = networkVideoSourceSystem->getNetworkVideoSourceById(videoSourceId);
		if (networkVideoSourcePtr)
		{
			return networkVideoSourcePtr;
		}

		auto usbVideoSourceSystem = projectManager->getSystemOfType<USBVideoSourceSystem>();
		auto usbVideoSourcePtr = usbVideoSourceSystem->getUSBVideoSourceById(videoSourceId);
		if (usbVideoSourcePtr)
		{
			return usbVideoSourcePtr;
		}

		return VideoSourceComponentPtr();
	}

	eVideoSourceType getVideoSourceType(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId)
	{
		auto networkVideoSourceSystem = projectManager->getSystemOfType<NetworkVideoSourceSystem>();
		auto networkVideoSourcePtr = networkVideoSourceSystem->getNetworkVideoSourceById(videoSourceId);
		if (networkVideoSourcePtr)
		{
			return eVideoSourceType::networked;
		}

		auto usbVideoSourceSystem = projectManager->getSystemOfType<USBVideoSourceSystem>();
		auto usbVideoSourcePtr = usbVideoSourceSystem->getUSBVideoSourceById(videoSourceId);
		if (usbVideoSourcePtr)
		{
			return eVideoSourceType::usb;
		}

		return eVideoSourceType::INVALID;
	}

	bool removeVideoSource(ProjectManagerConstPtr projectManager, MikanVideoSourceID videoSourceId)
	{
		switch (getVideoSourceType(projectManager, videoSourceId))
		{
			case eVideoSourceType::networked:
			{
				auto networkVideoSourceSystem = projectManager->getSystemOfType<NetworkVideoSourceSystem>();
				return networkVideoSourceSystem->removeNetworkVideoSource(videoSourceId);
			}
			case eVideoSourceType::usb:
			{
				auto usbVideoSourceSystem = projectManager->getSystemOfType<USBVideoSourceSystem>();
				return usbVideoSourceSystem->removeUSBVideoSource(videoSourceId);
			}
		}

		return false;
	}
}
