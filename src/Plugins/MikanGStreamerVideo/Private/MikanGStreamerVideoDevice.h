#pragma once

#include "INetworkVideoDevice.h"
#include "WorkerThread.h"

#include <array>
#include <future>
#include <string>
#include <set>

class MikanGStreamerVideoDevice : public INetworkVideoDevice
{
public:
	MikanGStreamerVideoDevice(class MikanGStreamerVideoDeviceManager* ownerDeviceManager,
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
	virtual eVideoOpeningStatus getVideoOpeningStatus() const override;
	virtual eVideoOpeningStatus open() override;
	virtual void close() override;

	// -- Video Settings
	virtual bool isVideoSettingSupported(const eVideoSettingType property_type) const override;
	virtual bool getVideoSettingConstraint(const eVideoSettingType property_type,
										   VideoSettingConstraint& outConstraint) const override;
	virtual void setVideoSetting(const eVideoSettingType property_type, int desired_value) override;
	virtual int getVideoSetting(const eVideoSettingType property_type) const override;

	// -- Video Streaming
	virtual eVideoStreamingStatus startVideoStream() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;

protected:
	void notifyVideoDeviceOpened();
	void notifyVideoDeviceClosed();
	void notifyVideoModePropertiesChanged();
	void notifyVideoFrameReceived(const NetworkVideoFrameBuffer& newBuffer);

private:
	bool openOnThread();

	enum class eOpenState
	{
		closed,
		opening,
		open,
		failed
	};

	static constexpr float k_streamTimeoutSeconds= 10.0f;

	class MikanGStreamerVideoDeviceManager* m_ownerDeviceManager;
	NetworkVideoConnectionSettings m_connectionInfo;
	NetworkVideoStreamProperties m_streamInfo;
	std::string m_url;
	struct GStreamerImpl* m_impl;
	eOpenState m_openState= eOpenState::closed;
	std::future<bool> m_openFuture;
	eVideoStreamingStatus m_streamingStatus= eVideoStreamingStatus::stopped;
	float m_timeSinceLastFrameSeconds= 0.0f;
	bool m_pendingStreamStartAfterOpen= false;

	std::set<INetworkVideoDeviceListener*> m_listeners;
};
