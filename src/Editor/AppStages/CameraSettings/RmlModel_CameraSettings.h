#pragma once

#include "MikanTypeFwd.h"
#include "ComponentFwd.h"
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

	bool init(
		Rml::Context* rmlContext, 
		VideoSourceSystemPtr videoSourceSystem,
		VideoSourceComponentPtr videoSourceComponent);
	virtual void dispose() override;

	inline RmlDataBinding_CameraBrightnessPtr getBrightnessDataBinding() const { return m_brightnessDataBinding; }
	void rebuildVideoSourceList();

	SinglecastDelegate<void(const MikanVideoSourceID videoSourceId)> OnUpdateVideoSourceId;

protected:
	void handleVideoSourceIdChanged(MikanVideoSourceID videoSourceId);

private:
	RmlDataBinding_CameraBrightnessPtr m_brightnessDataBinding;
	VideoSourceSystemWeakPtr m_videoSourceSystem;
	Rml::Vector<int> m_videoSourceIdList;
	MikanVideoSourceID m_videoSourceId;
};
