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

class VideoSourceDefinition : public MikanComponentDefinition
{
public:
	VideoSourceDefinition();
	VideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const std::string& videoSourceName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	inline MikanVideoSourceID getVideoSourceId() const { return m_videoSourceId; }

private:
	MikanVideoSourceID m_videoSourceId;
};

class USBVideoSourceDefinition : public VideoSourceDefinition
{
public:
	USBVideoSourceDefinition();
	USBVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const std::string& videoSourceName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_devicePathPropertyId;
	inline const std::string& getDevicePath() const { return m_devicePath; }
	void setDevicePath(const std::string& devicePath);

	static const std::string k_videoModePropertyId;
	inline const std::string& getVideoMode() const { return m_videoMode; }
	void setVideoMode(const std::string& videoMode);

	static const std::string k_brightnessPropertyId;
	inline float getBrightness() const { return m_brightness; }
	void setBrightness(const float brightness);

private:
	std::string m_devicePath;
	std::string m_videoMode;
	float m_brightness;
};

class NetworkVideoSourceDefinition : public VideoSourceDefinition
{
public:
	NetworkVideoSourceDefinition();
	NetworkVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const std::string& videoSourceName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_urlPropertyId;
	inline const std::string& getURL() const { return m_url; }
	void setURL(const std::string& url);

private:
	std::string m_url;
};

class SpoutVideoSourceDefinition : public VideoSourceDefinition
{
public:
	SpoutVideoSourceDefinition();
	SpoutVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const std::string& videoSourceName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_spoutSourcePropertyId;
	inline const std::string& getSpoutSource() const { return m_spoutSource; }
	void setSpoutSource(const std::string& spoutSource);

	static const std::string k_syncWithCameraPropertyId;
	inline bool getSyncWithCamera() const { return m_bSyncWithCamera; }
	void setSyncWithCamera(bool bSyncFlag);

private:
	std::string m_spoutSource;
	bool m_bSyncWithCamera;
};

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

	static const std::string k_usbVideoSourceListPropertyId;
	USBVideoSourceDefinitionConstPtr getUSBVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	USBVideoSourceDefinitionPtr getUSBVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addUSBVideoSource(const std::string& videoSourceName);
	bool removeUSBVideoSource(MikanVideoSourceID videoSourceId);

	static const std::string k_networkedVideoSourceListPropertyId;
	NetworkVideoSourceDefinitionConstPtr getNetworkedVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	NetworkVideoSourceDefinitionPtr getNetworkedVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addNetworkedVideoSource(const std::string& videoSourceName);
	bool removeNetworkedVideoSource(MikanVideoSourceID videoSourceId);

	static const std::string k_spoutVideoSourceListPropertyId;
	SpoutVideoSourceDefinitionConstPtr getSpoutVideoSourceConfigConst(MikanVideoSourceID videoSourceId) const;
	SpoutVideoSourceDefinitionPtr getSpoutVideoSourceConfig(MikanVideoSourceID videoSourceId);
	MikanVideoSourceID addSpoutVideoSource(const std::string& videoSourceName);
	bool removeSpoutVideoSource(MikanVideoSourceID videoSourceId);

protected:
	std::vector<USBVideoSourceDefinitionPtr> m_usbVideoSourceList;
	std::vector<NetworkVideoSourceDefinitionPtr> m_networkedVideoSourceList;
	std::vector<SpoutVideoSourceDefinitionPtr> m_spoutVideoSourceList;
	MikanVideoSourceID m_nextVideoSourceId = 0;
};