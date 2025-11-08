#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceSystemConfig.h"

#include <filesystem>
#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>

class VideoSourceSystem : public MikanObjectSystem
{
public:
	VideoSourceSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static VideoSourceSystemPtr getSystem() { return s_VideoSourceSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	VideoSourceSystemConfigConstPtr getVideoSourceSystemConfigConst() const;
	VideoSourceSystemConfigPtr getVideoSourceSystemConfig();

	VideoSourceIdList getVideoSourceIdList() const;
	VideoSourceComponentPtr getVideoSourceById(MikanVideoSourceID videoSourceId) const;
	eVideoSourceType getVideoSourceType(MikanVideoSourceID videoSourceId) const;
	bool removeVideoSource(MikanVideoSourceID videoSourceId);

	NetworkVideoSourceSystemPtr getNetworkVideoSourceSystem() const { return m_networkVideoSourceSystem; }
	USBVideoSourceSystemPtr getUSBVideoSourceSystem() const { return m_usbVideoSourceSystem; }

private:
	NetworkVideoSourceSystemPtr m_networkVideoSourceSystem;
	USBVideoSourceSystemPtr m_usbVideoSourceSystem;

	static VideoSourceSystemWeakPtr s_VideoSourceSystem;
};
