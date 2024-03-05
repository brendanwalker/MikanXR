#pragma once

#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlDataBinding_CameraBrightness;
using RmlDataBinding_CameraBrightnessPtr = std::shared_ptr<RmlDataBinding_CameraBrightness>;

class RmlModel_CameraSettings : public RmlModel
{
public:
	RmlModel_CameraSettings();

	bool init(Rml::Context* rmlContext,
			  ProfileConfigConstPtr profile,
			  const class VideoSourceManager* videoSourceManager);
	virtual void dispose() override;

	inline RmlDataBinding_CameraBrightnessPtr getBrightnessDataBinding() const { return m_brightnessDataBinding; }
	void rebuildVideoSourceList(const class VideoSourceManager* videoSourceManager);

	SinglecastDelegate<void(const std::string& devicePath)> OnUpdateVideoSourcePath;

protected:
	void handleVideoSourcePathChanged(const std::string& devicePath);

private:
	RmlDataBinding_CameraBrightnessPtr m_brightnessDataBinding;
	Rml::Vector<Rml::String> m_videoSourcePathList;
	Rml::String m_videoSourcePath;
};
