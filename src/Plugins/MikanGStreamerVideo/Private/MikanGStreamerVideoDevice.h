#pragma once

#include "INetworkVideoDevice.h"
#include "WorkerThread.h"

#include <array>
#include <chrono>
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

	// Tear down and re-open the pipeline, resuming streaming once the reopen completes.
	// Used both by the frame-timeout watchdog and the bus ERROR/EOS handler.
	void restartStream();

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
	// Wall-clock timestamp of the last received frame, used by the stall watchdog.
	// Deliberately wall-clock (steady_clock) rather than an accumulated deltaSeconds:
	// the App clamps deltaSeconds to 0.1s (App::tick), so a long freeze (e.g. a debugger
	// breakpoint) would otherwise take ~k_streamTimeoutSeconds of real time *after*
	// resuming to be detected. steady_clock keeps advancing while the thread is frozen,
	// so the stall is caught on the first tick after resuming.
	std::chrono::steady_clock::time_point m_lastFrameTimestamp;
	bool m_pendingStreamStartAfterOpen= false;

	std::set<INetworkVideoDeviceListener*> m_listeners;
};
