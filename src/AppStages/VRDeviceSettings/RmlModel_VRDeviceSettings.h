#pragma once

#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "FrameCompositorConstants.h"


class RmlModel_VRDeviceSettings : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext, 
			  ProfileConfigConstPtr profile,
			  const class VRDeviceManager* vrDeviceManager);
	virtual void dispose() override;

	void rebuildVRDeviceList(const class VRDeviceManager* vrDeviceManager);

	SinglecastDelegate<void(const std::string& devicePath)> OnUpdateCameraVRDevicePath;
	SinglecastDelegate<void(const std::string& devicePath)> OnUpdateMatVRDevicePath;

private:
	Rml::Vector<Rml::String> m_vrDeviceList;

	Rml::String m_cameraVRDevicePath;
	Rml::String m_matVRDevicePath;
};
