#pragma once

#include "ComponentFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"

class RmlDataBinding_VRDeviceList;
using RmlDataBinding_VRDeviceListPtr = std::shared_ptr<RmlDataBinding_VRDeviceList>;

class RmlModel_CompositorStages : public RmlModel
{
public:
	RmlModel_CompositorStages();

	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	//const Rml::String& getVideoSourceName() const;
	//void setVideoSourceName(const Rml::String& newName);

private:
	RmlDataBinding_VRDeviceListPtr m_vrDeviceBinding;
	ProjectConfigPtr m_projectConfigPtr;
	VideoSourceComponentPtr m_videoSource;

	// Cameras UI
	Rml::String m_cameraVRDevicePath;
	Rml::Vector<Rml::String> m_cameraNames;
};
