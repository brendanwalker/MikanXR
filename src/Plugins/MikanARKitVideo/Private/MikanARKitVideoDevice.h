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
// MikanGStreamerVideoDevice's async open/close/update pattern, but the video RTP
// pipeline itself is stubbed here - see openOnThread() - and deferred to Track C2.
// What IS real in this ticket (C1): depth (basePort+1) and pose (basePort+2)
// channel reception via ARKitDepthReceiver/ARKitPoseReceiver (tickets B4/B5) and
// their correlation via ARKitFrameCorrelator (ticket B6), started on open().
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

	ARKitDepthReceiver m_depthReceiver;
	ARKitPoseReceiver m_poseReceiver;
	ARKitFrameCorrelator m_frameCorrelator;

	std::mutex m_listenersMutex;
	std::set<IARKitVideoDeviceListener*> m_listeners;
};
