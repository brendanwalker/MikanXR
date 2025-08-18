#pragma once

#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlDataBinding_CameraBrightness;
using RmlDataBinding_CameraBrightnessPtr = std::shared_ptr<RmlDataBinding_CameraBrightness>;

class RmlModel_CameraSettings : public RmlModel
{
public:
	RmlModel_CameraSettings();

	bool init(Rml::Context* rmlContext, VideoSourceSystemPtr videoSourceManager);
	virtual void dispose() override;

	inline RmlDataBinding_CameraBrightnessPtr getBrightnessDataBinding() const { return m_brightnessDataBinding; }
	void rebuildVideoSourceList(VideoSourceSystemPtr videoSourceManager);

	SinglecastDelegate<void(const MikanVideoSourceID videoSourceId)> OnUpdateVideoSourceId;

protected:
	void handleVideoSourceIdChanged(MikanVideoSourceID videoSourceId);

private:
	RmlDataBinding_CameraBrightnessPtr m_brightnessDataBinding;
	Rml::Vector<int> m_videoSourceIdList;
	MikanVideoSourceID m_videoSourceId;
};
