#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <configuru.hpp>

class VideoSourceSystemConfig : public CommonConfig
{
public:
	VideoSourceSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eVideoSourceType getVideoSourceType(MikanVideoSourceID videoSourceId) const;
	bool removeVideoSource(MikanVideoSourceID videoSourceId);

	static const std::string k_clientVideoSourceListPropertyId;
	const std::vector<ClientVideoSourceDefinitionPtr>& getClientVideoSourceList() const { return m_clientVideoSourceList; }
	ClientVideoSourceDefinitionConstPtr getClientVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	ClientVideoSourceDefinitionPtr getClientVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addClientVideoSource(const struct MikanClientVideoSourceInfo& videoSourceInfo);
	bool removeClientVideoSource(MikanVideoSourceID videoSourceId);

	static const std::string k_networkedVideoSourceListPropertyId;
	const std::vector<NetworkVideoSourceDefinitionPtr>& getNetworkedVideoSourceList() const { return m_networkedVideoSourceList; }
	NetworkVideoSourceDefinitionConstPtr getNetworkedVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	NetworkVideoSourceDefinitionPtr getNetworkedVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addNetworkedVideoSource(const struct MikanNetworkVideoSourceInfo& videoSourceInfo);
	bool removeNetworkedVideoSource(MikanVideoSourceID videoSourceId);

	static const std::string k_spoutVideoSourceListPropertyId;
	const std::vector<SpoutVideoSourceDefinitionPtr>& getSpoutVideoSourceList() const { return m_spoutVideoSourceList; }
	SpoutVideoSourceDefinitionConstPtr getSpoutVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	SpoutVideoSourceDefinitionPtr getSpoutVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addSpoutVideoSource(const struct MikanSpoutVideoSourceInfo& videoSourceInfo);
	bool removeSpoutVideoSource(MikanVideoSourceID videoSourceId);

	static const std::string k_usbVideoSourceListPropertyId;
	const std::vector<USBVideoSourceDefinitionPtr>& getUSBVideoSourceList() const { return m_usbVideoSourceList; }
	USBVideoSourceDefinitionConstPtr getUSBVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	USBVideoSourceDefinitionPtr getUSBVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addUSBVideoSource(const struct MikanUSBVideoSourceInfo& videoSourceInfo);
	bool removeUSBVideoSource(MikanVideoSourceID videoSourceId);

protected:
	std::vector<ClientVideoSourceDefinitionPtr> m_clientVideoSourceList;
	std::vector<USBVideoSourceDefinitionPtr> m_usbVideoSourceList;
	std::vector<NetworkVideoSourceDefinitionPtr> m_networkedVideoSourceList;
	std::vector<SpoutVideoSourceDefinitionPtr> m_spoutVideoSourceList;
	MikanVideoSourceID m_nextVideoSourceId = 0;
};