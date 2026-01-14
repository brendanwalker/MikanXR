#include "TrackingVolumeQueries.h"
#include "MarkerTrackingVolumeSystem.h"
#include "MarkerTrackingVolumeComponent.h"
#include "ProjectManager.h"
#include "VRTrackingVolumeSystem.h"
#include "VRTrackingVolumeComponent.h"

namespace TrackingVolumeQueries
{
	TrackingVolumeIdList getTrackingVolumeIdList(ProjectManagerConstPtr projectManager)
	{
		TrackingVolumeIdList trackingVolumeIdList;

		auto markerTrackingVolumeSystem = projectManager->getSystemOfType<MarkerTrackingVolumeSystem>();
		auto markerTrackingVolumeIds = markerTrackingVolumeSystem->getTrackingVolumeIdList();
		trackingVolumeIdList.insert(trackingVolumeIdList.end(), markerTrackingVolumeIds.begin(), markerTrackingVolumeIds.end());

		auto vrTrackingVolumeSystem = projectManager->getSystemOfType<VRTrackingVolumeSystem>();
		auto vrTrackingVolumeIds = vrTrackingVolumeSystem->getTrackingVolumeIdList();
		trackingVolumeIdList.insert(trackingVolumeIdList.end(), vrTrackingVolumeIds.begin(), vrTrackingVolumeIds.end());

		return trackingVolumeIdList;
	}

	TrackingVolumeComponentPtr getTrackingVolumeById(ProjectManagerConstPtr projectManager, MikanTrackingVolumeID trackingVolumeId)
	{
		auto markerTrackingVolumeSystem = projectManager->getSystemOfType<MarkerTrackingVolumeSystem>();
		auto markerTrackingVolumePtr = markerTrackingVolumeSystem->getTypedComponentById(trackingVolumeId);
		if (markerTrackingVolumePtr)
		{
			return markerTrackingVolumePtr;
		}

		auto vrTrackingVolumeSystem = projectManager->getSystemOfType<VRTrackingVolumeSystem>();
		auto vrTrackingVolumePtr = vrTrackingVolumeSystem->getTypedComponentById(trackingVolumeId);
		if (vrTrackingVolumePtr)
		{
			return vrTrackingVolumePtr;
		}

		return TrackingVolumeComponentPtr();
	}

	eTrackingVolumeType getTrackingVolumeType(ProjectManagerConstPtr projectManager, MikanTrackingVolumeID trackingVolumeId)
	{
		auto markerTrackingVolumeSystem = projectManager->getSystemOfType<MarkerTrackingVolumeSystem>();
		auto markerTrackingVolumePtr = markerTrackingVolumeSystem->getTypedComponentById(trackingVolumeId);
		if (markerTrackingVolumePtr)
		{
			return eTrackingVolumeType::marker;
		}

		auto vrTrackingVolumeSystem = projectManager->getSystemOfType<VRTrackingVolumeSystem>();
		auto vrTrackingVolumePtr = vrTrackingVolumeSystem->getTypedComponentById(trackingVolumeId);
		if (vrTrackingVolumePtr)
		{
			return eTrackingVolumeType::vr;
		}

		return eTrackingVolumeType::INVALID;
	}

	bool removeTrackingVolume(ProjectManagerConstPtr projectManager, MikanTrackingVolumeID trackingVolumeId)
	{
		switch (getTrackingVolumeType(projectManager, trackingVolumeId))
		{
			case eTrackingVolumeType::marker:
			{
				auto markerTrackingVolumeSystem = projectManager->getSystemOfType<MarkerTrackingVolumeSystem>();
				return markerTrackingVolumeSystem->removeObject(trackingVolumeId);
			}
			case eTrackingVolumeType::vr:
			{
				auto vrTrackingVolumeSystem = projectManager->getSystemOfType<VRTrackingVolumeSystem>();
				return vrTrackingVolumeSystem->removeObject(trackingVolumeId);
			}
		}

		return false;
	}
}
