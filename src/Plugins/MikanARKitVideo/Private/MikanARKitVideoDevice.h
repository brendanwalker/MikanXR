#pragma once

#include "IARKitVideoDevice.h"
#include "ARKitDepthReceiver.h"
#include "ARKitPoseReceiver.h"
#include "ARKitFrameCorrelator.h"

#include <future>
#include <mutex>
#include <set>
#include <string>

// Implements IARKitVideoDevice (ticket B7). Structurally mirrors
// MikanGStreamerVideoDevice's async open/close/update pattern: depth (basePort+1)
// and pose (basePort+2) channel reception via ARKitDepthReceiver/ARKitPoseReceiver
// (tickets B4/B5) plus their correlation via ARKitFrameCorrelator (ticket B6) are
// started on open() (ticket C1); the video RTP receive pipeline itself
// (udpsrc/rtph264depay/decodebin, on basePort+0) is built in openOnThread() and
// polled in update() (ticket C2). ARKitRTPHeaderExtension (ticket C3) extracts the
// frameSeq/captureTimestampUs carried by each RTP packet's header extension and
// attaches it to the decoded buffer as ARKitFrameSeqMeta; update()'s appsink pull
// site reads that meta and feeds frameSeq/timestamp (but not yet decoded pixel
// data - see notifyFrameBundleReceived) into m_frameCorrelator.
class MikanARKitVideoDevice : public IARKitVideoDevice
{
public:
	MikanARKitVideoDevice(class MikanARKitVideoDeviceManager* ownerDeviceManager,
						  const ARKitVideoConnectionSettings& connectionInfo);
	virtual ~MikanARKitVideoDevice();

	virtual void update(float deltaSeconds) override;

	// -- Device Listener
	virtual void addListener(IARKitVideoDeviceListener* listener) override;
	virtual void removeListener(IARKitVideoDeviceListener* listener) override;

	// -- Device Properties
	virtual const char* getDevicePath() const override;
	virtual const char* getFriendlyName() const override;

	// -- Device Activation
	virtual eVideoOpeningStatus getVideoOpeningStatus() const override;
	virtual eVideoOpeningStatus open() override;
	virtual void close() override;

	// -- Video Settings (not applicable over this transport, matching
	// MikanGStreamerVideoDevice's equivalent stubs)
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
	bool openOnThread();

	// Converts the plugin-private ARKitFrameBundle (ARKitFrameCorrelator) into the
	// public, DLL-boundary-safe ARKitVideoFrameBundle (IARKitVideoDevice) and fans
	// it out to listeners. NOTE: this can be invoked from ARKitDepthReceiver's or
	// ARKitPoseReceiver's worker thread (whichever notify*() call finalized the
	// bundle), not necessarily the thread that calls update() - listeners must
	// treat notifyFrameBundleReceived the same way those receivers' own callbacks
	// are documented: safe to call from a background thread.
	void notifyFrameBundleReceived(const ARKitFrameBundle& internalBundle);

private:
	enum class eOpenState
	{
		closed,
		opening,
		open,
		failed
	};

	class MikanARKitVideoDeviceManager* m_ownerDeviceManager;
	ARKitVideoConnectionSettings m_connectionInfo;
	std::string m_devicePath;

	eOpenState m_openState= eOpenState::closed;
	std::future<bool> m_openFuture;
	eVideoStreamingStatus m_streamingStatus= eVideoStreamingStatus::stopped;

	// Opaque GStreamer pipeline state (pipeline/appsink/bus). Defined in the .cpp so
	// this header stays free of GStreamer includes, matching
	// MikanGStreamerVideoDevice.h's m_impl pattern.
	struct GStreamerImpl* m_impl;

	static constexpr float k_streamTimeoutSeconds= 10.0f;
	float m_timeSinceLastFrameSeconds= 0.0f;
	bool m_pendingStreamStartAfterOpen= false;

	ARKitDepthReceiver m_depthReceiver;
	ARKitPoseReceiver m_poseReceiver;
	ARKitFrameCorrelator m_frameCorrelator;

	std::mutex m_listenersMutex;
	std::set<IARKitVideoDeviceListener*> m_listeners;
};
