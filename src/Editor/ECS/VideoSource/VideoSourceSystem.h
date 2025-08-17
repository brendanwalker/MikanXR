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

// Forward declarations
class ClientVideoSourceSystem;
class NetworkVideoSourceSystem;
class SpoutVideoSourceSystem;
class USBVideoSourceSystem;

using ClientVideoSourceSystemPtr = std::shared_ptr<ClientVideoSourceSystem>;
using NetworkVideoSourceSystemPtr = std::shared_ptr<NetworkVideoSourceSystem>;
using SpoutVideoSourceSystemPtr = std::shared_ptr<SpoutVideoSourceSystem>;
using USBVideoSourceSystemPtr = std::shared_ptr<USBVideoSourceSystem>;

class VideoSourceSystem : public MikanObjectSystem
{
public:
	VideoSourceSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static VideoSourceSystemPtr getSystem() { return s_VideoSourceSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	VideoSourceSystemConfigConstPtr getVideoSourceSystemConfigConst() const;
	VideoSourceSystemConfigPtr getVideoSourceSystemConfig();

	CameraComponentPtr getCurrentSceneCameraComponent() const;
	VideoSourceComponentPtr getCurrentSceneVideoSource() const;
	VideoSourceComponentPtr getVideoSourceById(MikanVideoSourceID videoSourceId) const;
	eVideoSourceType getVideoSourceType(MikanVideoSourceID videoSourceId) const;
	bool removeVideoSource(MikanVideoSourceID videoSourceId);

	// Client Video Source Methods - delegate to ClientVideoSourceSystem
	ClientVideoSourceSystemPtr getClientVideoSourceSystem() const { return m_clientVideoSourceSystem; }
	NetworkVideoSourceSystemPtr getNetworkVideoSourceSystem() const { return m_networkVideoSourceSystem; }
	SpoutVideoSourceSystemPtr getSpoutVideoSourceSystem() const { return m_spoutVideoSourceSystem; }
	USBVideoSourceSystemPtr getUSBVideoSourceSystem() const { return m_usbVideoSourceSystem; }

private:
	ClientVideoSourceSystemPtr m_clientVideoSourceSystem;
	NetworkVideoSourceSystemPtr m_networkVideoSourceSystem;
	SpoutVideoSourceSystemPtr m_spoutVideoSourceSystem;
	USBVideoSourceSystemPtr m_usbVideoSourceSystem;

	static VideoSourceSystemWeakPtr s_VideoSourceSystem;
};
