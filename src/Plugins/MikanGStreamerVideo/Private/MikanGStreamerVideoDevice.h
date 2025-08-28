#pragma once

#include "INetworkVideoDevice.h"
#include "WorkerThread.h"

#include <array>
#include <string>
#include <set>


class MikanGStreamerVideoDevice : public INetworkVideoDevice
{
public:
	MikanGStreamerVideoDevice(
		class MikanGStreamerVideoDeviceManager* ownerDeviceManager,
		const NetworkVideoConnectionSettings& connectionInfo);
	virtual ~MikanGStreamerVideoDevice();

	static std::string constructURL(const NetworkVideoConnectionSettings& connectionInfo);

	virtual void update(float deltaSeconds) override;

	// -- Device Listener
	virtual void addListener(INetworkVideoDeviceListener* listener) override;
	virtual void removeListener(INetworkVideoDeviceListener* listener) override;

	// -- Device Properties
	virtual const char* getDevicePath() const override;
	virtual const char* getFriendlyName() const override;
	virtual bool getStreamProperties(NetworkVideoStreamProperties& outProperties) const override;

	// -- Device Activation
	virtual bool getIsOpen() const override;
	virtual bool open() override;
	virtual void close() override;

	// -- Video Settings
	virtual bool isVideoSettingSupported(const eVideoSettingType property_type) const override;
	virtual bool getVideoSettingConstraint(
		const eVideoSettingType property_type, 
		VideoSettingConstraint& outConstraint) const override;
	virtual void setVideoSetting(const eVideoSettingType property_type, int desired_value) override;
	virtual int getVideoSetting(const eVideoSettingType property_type) const override;

	// -- Video Streaming
	virtual eVideoStreamingStatus startVideoStream() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;
	
protected:
	void notifyVideoDeviceClosed();
	void notifyVideoModePropertiesChanged();
	void notifyVideoFrameReceived(const NetworkVideoFrameBuffer& newBuffer);

private:
	class MikanGStreamerVideoDeviceManager* m_ownerDeviceManager;
	NetworkVideoConnectionSettings m_connectionInfo;
	NetworkVideoStreamProperties m_streamInfo;
	std::string m_url;
	struct GStreamerImpl* m_impl;
	bool m_bIsStreaming;

	std::set<INetworkVideoDeviceListener*> m_listeners;
};

