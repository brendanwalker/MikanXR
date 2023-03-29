#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"


class RmlModel_VRDeviceSettings : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, 
			  const class ProfileConfig* profile,
			  const class VRDeviceManager* vrDeviceManager);
	virtual void dispose() override;

	void rebuildVRDeviceList(const class VRDeviceManager* vrDeviceManager);
	void rebuildAnchorList(const class ProfileConfig* profile);

	SinglecastDelegate<void(const std::string& devicePath)> OnUpdateCameraVRDevicePath;
	SinglecastDelegate<void(int anchorId)> OnUpdateCameraParentAnchorId;
	SinglecastDelegate<void(const std::string& devicePath)> OnUpdateMatVRDevicePath;
	SinglecastDelegate<void(const std::string& devicePath)> OnUpdateOriginVRDevicePath;
	SinglecastDelegate<void(const float newScale)> OnUpdateCameraScale;

private:
	Rml::Vector<Rml::String> m_vrDeviceList;
	Rml::Vector<int> m_spatialAnchors;

	Rml::String m_cameraVRDevicePath;
	int m_cameraParentAnchorId= -1;
	float m_cameraScale = 1.f;
	Rml::String m_matVRDevicePath;
	Rml::String m_originVRDevicePath;
};
