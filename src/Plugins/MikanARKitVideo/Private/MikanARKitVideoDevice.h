#pragma once

#include "IARKitVideoDevice.h"
#include "ARKitDepthReceiver.h"
#include "ARKitMatteReceiver.h"
#include "ARKitPoseReceiver.h"
#include "ARKitFrameCorrelator.h"
#include "Cuda/CudaGLInterop.h"
#include "Cuda/JBUKernel.h"
#include "Cuda/NV12ConversionKernel.h"

#include <atomic>
#include <future>
#include <mutex>
#include <optional>
#include <set>
#include <string>

// Forward declaration matching GStreamer's own typedef (gst/gstbuffer.h) - kept
// opaque here so this header doesn't need to pull in gst.h, same rationale as
// GStreamerImpl below.
typedef struct _GstBuffer GstBuffer;

// Implements IARKitVideoDevice (ticket B7). Structurally mirrors
// MikanGStreamerVideoDevice's async open/close/update pattern: depth (basePort+1)
// and pose (basePort+2) channel reception via ARKitDepthReceiver/ARKitPoseReceiver
// (tickets B4/B5) plus their correlation via ARKitFrameCorrelator (ticket B6) are
// started on open() (ticket C1); the video RTP receive pipeline itself
// (udpsrc/rtph264depay/nvh264dec, on basePort+0) is built in openOnThread() and
// polled in update() (tickets C2/C4 - hardware-decodes straight to CUDA device
// memory, no software fallback). ARKitRTPHeaderExtension (ticket C3) extracts the
// frameSeq/captureTimestampUs carried by each RTP packet's header extension and
// attaches it to the decoded buffer as ARKitFrameSeqMeta; update()'s appsink pull
// site reads that meta and maps the buffer as CUDA memory (GST_MAP_CUDA) to get a
// CUdeviceptr, feeding frameSeq/timestamp (but not yet the device pointer itself -
// see notifyFrameBundleReceived) into m_frameCorrelator.
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

	// -- Zero-copy CUDA-GL texture access (ticket E3)
	virtual uint32_t getColorTextureGlId() const override;
	virtual uint32_t getDepthTextureGlId() const override;
	virtual uint32_t getHumanStencilRefinedTextureGlId() const override;
	virtual uint32_t getHumanStencilRawTextureGlId() const override;

	// -- JBU Tuning
	virtual void setJBUParams(const ARKitJBUParams& params) override;

	// -- Depth preview mode (transient debug view - see IARKitVideoDevice)
	virtual void setDepthPreviewMode(eARKitDepthPreviewMode mode) override;

protected:
	bool openOnThread();

	// Called from update()'s appsink-pull site once per successfully-decoded CUDA
	// frame (ticket E3). Resizes the CUDA-GL textures/device buffers on dimension
	// change, runs the NV12->RGB(A) conversion kernel (writing the display color
	// texture + JBU's guide-RGB buffer), uploads the latest cached depth+confidence,
	// and runs JBUKernel::upsampleToSurface into the depth texture. Must run on the
	// GL-context-owning thread (same as update() itself already does - see
	// CudaGLInterop.h's threading note).
	void updateColorAndDepthTextures(GstBuffer* buffer, CUdeviceptr devicePtr, int width, int height);
	bool ensureTexturePipelineInitialized(int width, int height);

	// Live-tested finding (ticket E3): contrary to JBUKernel.h's original design
	// note (which assumed upsample() could just reuse whatever CUDA context
	// nvcodec's pipeline already has current), no context is actually current on
	// this thread by the time update() runs cuGraphicsGLRegisterImage/kernel
	// launches here - GST_MAP_CUDA's own context handling (if any) doesn't leave
	// one current for callers afterward (confirmed via a live
	// CUDA_ERROR_INVALID_CONTEXT failure). So this class creates and owns its own
	// CUcontext instead, independent of nvcodec's internal GstCudaContext - safe
	// because CUDA's Unified Virtual Addressing (the default on 64-bit
	// platforms/modern CUDA) makes device pointers allocated under one context on a
	// device valid to dereference from another context on that same device, within
	// the same process - which is exactly what's needed to read a GST_MAP_CUDA
	// pointer from our own context. Lazily created once; cuCtxSetCurrent() is
	// re-asserted defensively at the start of every updateColorAndDepthTextures()
	// call since anything else that ran on the main thread in between (including
	// GStreamer's own internal context push/pop during gst_buffer_map) could have
	// changed what's current.
	bool ensureCudaContext();

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
	ARKitMatteReceiver m_matteReceiver;
	ARKitPoseReceiver m_poseReceiver;
	ARKitFrameCorrelator m_frameCorrelator;

	std::mutex m_listenersMutex;
	std::set<IARKitVideoDeviceListener*> m_listeners;

	// -- Zero-copy CUDA-GL texture pipeline (ticket E3) --
	// Latest depth+confidence sample, updated from ARKitDepthReceiver's callback
	// (background thread) and read from updateColorAndDepthTextures() (GL/main
	// thread) under m_latestDepthMutex. Deliberately independent of
	// m_frameCorrelator's frameSeq-exact bundle matching (whose ~1s stale-sweep
	// timeout suits pose/depth bookkeeping, not a tight real-time compositing
	// loop) - JBU runs against whichever depth sample is freshest each time a new
	// color frame decodes, tolerating a few ms of depth/color skew the same way any
	// real-time occlusion compositor does.
	std::mutex m_latestDepthMutex;
	std::optional<ARKitDepthFrame> m_latestDepthFrame;

	// Latest native-resolution person matte, updated from ARKitMatteReceiver's callback
	// (background thread) and read from updateColorAndDepthTextures() (GL/main thread)
	// under m_latestMatteMutex. Same freshest-wins policy as the depth cache above - the
	// matte rides its own basePort+3 channel now (not the depth payload), so it's carried
	// separately here rather than folded into ARKitDepthFrame.
	std::mutex m_latestMatteMutex;
	std::optional<ARKitMatteFrame> m_latestMatteFrame;

	bool m_bTexturePipelineInitialized= false;
	int m_textureWidth= 0;
	int m_textureHeight= 0;

	// This class's own CUDA context (see ensureCudaContext()'s comment for why) -
	// independent of whatever context nvcodec's pipeline owns internally.
	CUcontext m_cudaContext= nullptr;

	CudaGLColorTexture m_colorTexture;
	CudaGLDepthTexture m_depthTexture;
	// Full-res R32F person alpha textures exposed as compositor inputs (see
	// getHumanStencilRefinedTextureGlId/getHumanStencilRawTextureGlId). Refined = guided
	// stencil-JBU (crisp); Raw = nearest-upsampled 0/1 stencil (blocky ground truth).
	CudaGLDepthTexture m_humanStencilRefinedTexture;
	CudaGLDepthTexture m_humanStencilRawTexture;
	NV12ConversionKernel m_nv12Kernel;
	JBUKernel m_jbuKernel;

	// Live-tunable JBU params (see IARKitVideoDevice::setJBUParams). Defaults to
	// JBUParams' own D5-tuned defaults until a caller pushes real values (see
	// ARKitVideoSourceComponent::pushJBUParamsToDevice(), called once on device-open
	// and again on every subsequent live edit). Guarded by m_jbuParamsMutex since
	// setJBUParams() can be called from whatever thread drives property changes
	// (Editor GUI or a remote client's property-set RPC), while
	// updateColorAndDepthTextures() reads it from the GL/main thread.
	std::mutex m_jbuParamsMutex;
	JBUParams m_jbuParams;

	// Transient depth-preview selector (see setDepthPreviewMode). A plain atomic rather
	// than folding into m_jbuParamsMutex - it's a single scalar written from the GUI
	// thread and read on the GL/main thread, independent of the JBU tuning params.
	std::atomic<int> m_depthPreviewMode{static_cast<int>(eARKitDepthPreviewMode::jbu_upsampled)};

	// Full-resolution packed-RGB guide buffer for JBUKernel (written by
	// m_nv12Kernel, read by m_jbuKernel) - sized m_textureHeight * m_guideStrideBytes.
	CUdeviceptr m_guideRgbBuffer= 0;
	int m_guideStrideBytes= 0;

	// Low-res (kARKitDepthWidth x kARKitDepthHeight) device buffers the latest
	// cached depth/confidence are uploaded into each frame before the JBU launch.
	CUdeviceptr m_depthLowBuffer= 0;
	CUdeviceptr m_confidenceLowBuffer= 0;

	// Native-resolution person matte (0/1 labels), uploaded from the latest-matte cache
	// each frame when available. Feeds JBUKernel's guide-resolution silhouette gating and
	// the matte debug preview. Dynamically sized: the matte's dimensions are device-
	// dependent and travel on the wire (unlike depth's fixed 256x192 grid), so the buffer
	// is (re)allocated to fit whenever the incoming dimensions grow.
	CUdeviceptr m_matteBuffer= 0;
	size_t m_matteBufferCapacity= 0;
	int m_matteWidth= 0;
	int m_matteHeight= 0;
};
