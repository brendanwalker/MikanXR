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

using VideoSourceIdList = std::vector<MikanVideoSourceID>;

class VideoSourceSystemConfig : public MikanObjectSystemDefinition
{
public:
	VideoSourceSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	eVideoSourceType getVideoSourceType(MikanVideoSourceID videoSourceId) const;
	bool removeVideoSource(MikanVideoSourceID videoSourceId);

	static const std::string k_networkedVideoSourceListPropertyId;
	const std::vector<NetworkVideoSourceDefinitionPtr>& getNetworkedVideoSourceList() const { return m_networkedVideoSourceList; }
	NetworkVideoSourceDefinitionConstPtr getNetworkedVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	NetworkVideoSourceDefinitionPtr getNetworkedVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addNetworkedVideoSource(const struct MikanNetworkVideoSourceInfo& videoSourceInfo);
	bool removeNetworkedVideoSource(MikanVideoSourceID videoSourceId);

	static const std::string k_usbVideoSourceListPropertyId;
	const std::vector<USBVideoSourceDefinitionPtr>& getUSBVideoSourceList() const { return m_usbVideoSourceList; }
	USBVideoSourceDefinitionConstPtr getUSBVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	USBVideoSourceDefinitionPtr getUSBVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addUSBVideoSource(const struct MikanUSBVideoSourceInfo& videoSourceInfo);
	bool removeUSBVideoSource(MikanVideoSourceID videoSourceId);

protected:
	std::vector<USBVideoSourceDefinitionPtr> m_usbVideoSourceList;
	std::vector<NetworkVideoSourceDefinitionPtr> m_networkedVideoSourceList;
	MikanVideoSourceID m_nextVideoSourceId = 0;
};