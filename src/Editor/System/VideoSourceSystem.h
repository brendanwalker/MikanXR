#pragma once

#include "ComponentFwd.h"
#include "MikanObjectSystem.h"
#include "MikanVideoSourceTypes.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "VideoSourceSystemConfig.h"

#include <filesystem>
#include <map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

using ClientVideoSourceMap = std::map<MikanVideoSourceID, ClientVideoSourceComponentWeakPtr>;
using SpoutVideoSourceMap = std::map<MikanVideoSourceID, SpoutVideoSourceComponentWeakPtr>;
using NetworkVideoSourceMap = std::map<MikanVideoSourceID, NetworkVideoSourceComponentWeakPtr>;
using USBVideoSourceMap = std::map<MikanVideoSourceID, USBVideoSourceComponentWeakPtr>;

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

	VideoSourceComponentPtr getVideoSourceById(MikanVideoSourceID videoSourceId) const;
	eVideoSourceType getVideoSourceType(MikanVideoSourceID videoSourceId) const;
	bool removeVideoSource(MikanVideoSourceID videoSourceId);

	const ClientVideoSourceMap& getClientVideoSourceMap() const { return m_clientVideoSourceComponents; }
	ClientVideoSourceComponentPtr getClientVideoSourceById(MikanVideoSourceID videoSourceId) const;
	ClientVideoSourceComponentPtr getClientVideoSourceByName(const std::string& videoSourceName) const;
	ClientVideoSourceComponentPtr addNewClientVideoSource(const MikanClientVideoSourceInfo& videoSourceInfo);
	bool removeClientVideoSource(MikanVideoSourceID videoSourceId);

	const NetworkVideoSourceMap& getNetworkVideoSourceMap() const { return m_networkVideoSourceComponents; }
	NetworkVideoSourceComponentPtr getNetworkVideoSourceById(MikanVideoSourceID videoSourceId) const;
	NetworkVideoSourceComponentPtr getNetworkVideoSourceByName(const std::string& videoSourceName) const;
	NetworkVideoSourceComponentPtr addNewNetworkVideoSource(const MikanNetworkVideoSourceInfo& videoSourceInfo);
	bool removeNetworkVideoSource(MikanVideoSourceID videoSourceId);

	const SpoutVideoSourceMap& getSpoutVideoSourceMap() const { return m_spoutVideoSourceComponents; }
	SpoutVideoSourceComponentPtr getSpoutVideoSourceById(MikanVideoSourceID videoSourceId) const;
	SpoutVideoSourceComponentPtr getSpoutVideoSourceByName(const std::string& videoSourceName) const;
	SpoutVideoSourceComponentPtr addNewSpoutVideoSource(const MikanSpoutVideoSourceInfo& videoSourceInfo);
	bool removeSpoutVideoSource(MikanVideoSourceID videoSourceId);

	const USBVideoSourceMap& getUSBVideoSourceMap() const { return m_usbVideoSourceComponents; }
	USBVideoSourceComponentPtr getUSBVideoSourceById(MikanVideoSourceID videoSourceId) const;
	USBVideoSourceComponentPtr getUSBVideoSourceByName(const std::string& videoSourceName) const;
	USBVideoSourceComponentPtr addNewUSBVideoSource(const MikanUSBVideoSourceInfo& videoSourceInfo);
	bool removeUSBVideoSource(MikanVideoSourceID videoSourceId);

protected:
	ClientVideoSourceComponentPtr createClientVideoSourceObject(ClientVideoSourceDefinitionPtr sourceConfig);
	bool disposeClientVideoSourceObject(MikanVideoSourceID videoSourceId);

	NetworkVideoSourceComponentPtr createNetworkVideoSourceObject(NetworkVideoSourceDefinitionPtr sourceConfig);
	bool disposeNetworkVideoSourceObject(MikanVideoSourceID videoSourceId);

	SpoutVideoSourceComponentPtr createSpoutVideoSourceObject(SpoutVideoSourceDefinitionPtr sourceConfig);
	bool disposeSpoutVideoSourceObject(MikanVideoSourceID videoSourceId);

	USBVideoSourceComponentPtr createUSBVideoSourceObject(USBVideoSourceDefinitionPtr sourceConfig);
	bool disposeUSBVideoSourceObject(MikanVideoSourceID videoSourceId);

	ClientVideoSourceMap m_clientVideoSourceComponents;
	NetworkVideoSourceMap m_networkVideoSourceComponents;
	SpoutVideoSourceMap m_spoutVideoSourceComponents;
	USBVideoSourceMap m_usbVideoSourceComponents;

	static VideoSourceSystemWeakPtr s_VideoSourceSystem;
};
