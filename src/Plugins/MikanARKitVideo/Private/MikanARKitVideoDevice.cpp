#include "MikanARKitVideoDevice.h"
#include "MikanARKitVideoDeviceManager.h"
#include "Logger.h"

#include <chrono>
#include <cstring>
#include <thread>

MikanARKitVideoDevice::MikanARKitVideoDevice(MikanARKitVideoDeviceManager* ownerDeviceManager,
											 const ARKitVideoConnectionSettings& connectionInfo)
	: m_ownerDeviceManager(ownerDeviceManager)
	, m_connectionInfo(connectionInfo)
	, m_devicePath("arkit://" + std::to_string(connectionInfo.basePort))
	, m_frameCorrelator(connectionInfo.depthStreamingEnabled)
{
	m_frameCorrelator.setBundleCallback([this](ARKitFrameBundle bundle) { notifyFrameBundleReceived(bundle); });

	m_depthReceiver.setFrameCallback([this](ARKitDepthFrame frame)
									 { m_frameCorrelator.notifyDepthArrived(std::move(frame)); });

	m_poseReceiver.setFrameCallback([this](ARKitPoseFrame frame)
									{ m_frameCorrelator.notifyPoseArrived(std::move(frame)); });
}

MikanARKitVideoDevice::~MikanARKitVideoDevice() { close(); }

// -- Device Listener -----
void MikanARKitVideoDevice::addListener(IARKitVideoDeviceListener* listener)
{
	std::lock_guard<std::mutex> lock(m_listenersMutex);
	m_listeners.insert(listener);
}

void MikanARKitVideoDevice::removeListener(IARKitVideoDeviceListener* listener)
{
	std::lock_guard<std::mutex> lock(m_listenersMutex);
	m_listeners.erase(listener);
}

// -- Device Properties -----
const char* MikanARKitVideoDevice::getDevicePath() const { return m_devicePath.c_str(); }

const char* MikanARKitVideoDevice::getFriendlyName() const { return getDevicePath(); }

// -- Device Activation -----
eVideoOpeningStatus MikanARKitVideoDevice::open()
{
	if (m_openState == eOpenState::open || m_openState == eOpenState::opening)
		return getVideoOpeningStatus();

	// Depth/pose sockets bind quickly (a single non-blocking recvfrom-socket bind
	// call each - see UdpReceiveSocket::open), unlike GStreamer pipeline
	// construction (which genuinely needs backgrounding - see openOnThread()), so
	// start them synchronously here rather than deferring to the background thread.
	if (m_connectionInfo.depthStreamingEnabled)
	{
		const uint16_t depthPort= static_cast<uint16_t>(m_connectionInfo.basePort + 1);
		if (!m_depthReceiver.start(depthPort))
		{
			MIKAN_LOG_ERROR("MikanARKitVideoDevice::open") << "Failed to start depth receiver on port " << depthPort;
			m_openState= eOpenState::failed;
			return eVideoOpeningStatus::failed;
		}
	}

	const uint16_t posePort= static_cast<uint16_t>(m_connectionInfo.basePort + 2);
	if (!m_poseReceiver.start(posePort))
	{
		MIKAN_LOG_ERROR("MikanARKitVideoDevice::open") << "Failed to start pose receiver on port " << posePort;
		m_depthReceiver.stop();
		m_openState= eOpenState::failed;
		return eVideoOpeningStatus::failed;
	}

	// Launch the (eventually GStreamer-based - see Track C2) video pipeline setup
	// on a background thread, matching MikanGStreamerVideoDevice's rationale
	// (pipeline construction is slow on first call - loads GStreamer DLLs).
	m_openState= eOpenState::opening;
	auto promise= std::make_shared<std::promise<bool>>();
	m_openFuture= promise->get_future();
	std::thread([promise, this]() mutable { promise->set_value(openOnThread()); }).detach();

	return eVideoOpeningStatus::opening;
}

bool MikanARKitVideoDevice::openOnThread()
{
	// TODO(Track C2): build the real udpsrc/rtph264depay/h264parse/nvh264dec RTP
	// video pipeline here (see MikanGStreamerVideoDevice::openOnThread() for the
	// GStreamer pipeline construction pattern this will follow, and
	// ARKitWireProtocol.h for the RTP header extension carrying frameSeq that will
	// feed ARKitFrameCorrelator::notifyVideoArrived). Stubbed to trivially succeed
	// for now, per ticket C1's scope - this lets ARKitVideoSourceSystem's
	// module-loading scaffold (ticket B8) exercise a real open/close cycle against
	// this plugin before the video path exists.
	return true;
}

eVideoOpeningStatus MikanARKitVideoDevice::getVideoOpeningStatus() const
{
	switch (m_openState)
	{
	case eOpenState::open:
		return eVideoOpeningStatus::open;
	case eOpenState::opening:
		return eVideoOpeningStatus::opening;
	case eOpenState::failed:
		return eVideoOpeningStatus::failed;
	default:
		return eVideoOpeningStatus::closed;
	}
}

void MikanARKitVideoDevice::update(float deltaSeconds)
{
	// Poll the async open future
	if (m_openState == eOpenState::opening)
	{
		if (m_openFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
		{
			const bool bSuccess= m_openFuture.get();
			if (bSuccess)
			{
				m_openState= eOpenState::open;

				std::lock_guard<std::mutex> lock(m_listenersMutex);
				for (IARKitVideoDeviceListener* listener : m_listeners)
					listener->notifyDeviceOpened(this);
			}
			else
			{
				m_openState= eOpenState::failed;
				m_depthReceiver.stop();
				m_poseReceiver.stop();
			}
		}
		return;
	}

	if (m_openState != eOpenState::open)
		return;

	// ARKitDepthReceiver/ARKitPoseReceiver each drive their own worker thread
	// already; the correlator is the one piece that needs an explicit periodic
	// tick to evict stale (incomplete) pending frame bundles - see
	// ARKitFrameCorrelator::sweepStaleFrames. Until Track C2 lands, no video
	// notifications ever arrive, so bundles only ever complete via this stale-sweep
	// path (depth+pose, or pose alone if depth streaming is disabled) - expected
	// for this ticket's interim state.
	m_frameCorrelator.sweepStaleFrames(std::chrono::steady_clock::now());
}

void MikanARKitVideoDevice::close()
{
	if (m_openState == eOpenState::opening && m_openFuture.valid())
	{
		// Block until the in-flight async open completes so we can safely clean up
		m_openFuture.get();
		// Don't notify — the device was never fully open from the caller's perspective
	}
	else if (m_openState == eOpenState::open)
	{
		std::lock_guard<std::mutex> lock(m_listenersMutex);
		for (IARKitVideoDeviceListener* listener : m_listeners)
			listener->notifyDeviceClosed(this);

		// Clients need to reregister as listeners when they reopen
		m_listeners.clear();
	}

	m_openState= eOpenState::closed;
	m_streamingStatus= eVideoStreamingStatus::stopped;

	m_depthReceiver.stop();
	m_poseReceiver.stop();

	// TODO(Track C2): tear down the real GStreamer pipeline here.
}

// -- Video Settings -----
bool MikanARKitVideoDevice::isVideoSettingSupported(const eVideoSettingType) const { return false; }

bool MikanARKitVideoDevice::getVideoSettingConstraint(const eVideoSettingType, VideoSettingConstraint&) const
{
	return false;
}

void MikanARKitVideoDevice::setVideoSetting(const eVideoSettingType, int)
{
	// Not supported
}

int MikanARKitVideoDevice::getVideoSetting(const eVideoSettingType) const { return -1; }

// -- Video Streaming -----
eVideoStreamingStatus MikanARKitVideoDevice::startVideoStream()
{
	if (m_openState == eOpenState::open)
	{
		m_streamingStatus= eVideoStreamingStatus::started;
		return m_streamingStatus;
	}

	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus MikanARKitVideoDevice::getVideoStreamingStatus() const { return m_streamingStatus; }

void MikanARKitVideoDevice::stopVideoStream() { m_streamingStatus= eVideoStreamingStatus::stopped; }

// -- Frame bundle conversion -----
void MikanARKitVideoDevice::notifyFrameBundleReceived(const ARKitFrameBundle& internalBundle)
{
	ARKitVideoFrameBundle publicBundle;
	publicBundle.frameSeq= internalBundle.frameSeq;
	publicBundle.timestampUs= internalBundle.timestampUs;

	publicBundle.hasVideo= internalBundle.hasVideo;
	publicBundle.videoData= nullptr; // TODO(Track C2): populate once real decoded frames exist
	publicBundle.videoWidth= 0;
	publicBundle.videoHeight= 0;

	if (internalBundle.depth.has_value())
	{
		publicBundle.hasDepth= true;
		publicBundle.depth.width= kARKitDepthWidth;
		publicBundle.depth.height= kARKitDepthHeight;
		publicBundle.depth.depthMM= internalBundle.depth->depthMM.data();
		publicBundle.depth.confidence= internalBundle.depth->confidence.data();
	}

	if (internalBundle.pose.has_value())
	{
		publicBundle.hasPose= true;
		std::memcpy(publicBundle.pose.transform, internalBundle.pose->transform, sizeof(publicBundle.pose.transform));
		publicBundle.pose.fx= internalBundle.pose->fx;
		publicBundle.pose.fy= internalBundle.pose->fy;
		publicBundle.pose.cx= internalBundle.pose->cx;
		publicBundle.pose.cy= internalBundle.pose->cy;
		publicBundle.pose.imageWidth= internalBundle.pose->imageWidth;
		publicBundle.pose.imageHeight= internalBundle.pose->imageHeight;
	}

	std::lock_guard<std::mutex> lock(m_listenersMutex);
	for (IARKitVideoDeviceListener* listener : m_listeners)
		listener->notifyFrameBundleReceived(publicBundle);
}
