#pragma once

#include "IARKitVideoDevice.h"
#include "Cuda/CudaGLInterop.h"
#include "Cuda/NV12ConversionKernel.h"

#include <future>
#include <mutex>
#include <set>
#include <string>

// Forward declaration matching GStreamer's own typedef (gst/gstbuffer.h) - kept
// opaque here so this header doesn't need to pull in gst.h, same rationale as
// GStreamerImpl below.
typedef struct _GstBuffer GstBuffer;

// Implements IARKitVideoDevice (ticket B7). Structurally mirrors
// MikanGStreamerVideoDevice's async open/close/update pattern: the video RTP
// receive pipeline (udpsrc/rtph264depay/nvh264dec, on basePort+0) is built in
// openOnThread() and polled in update() (tickets C2/C4 - hardware-decodes
// straight to CUDA device memory, no software fallback). ARKitRTPHeaderExtension
// (ticket C3) extracts the frameSeq/captureTimestampUs (and, when the sender
// attaches it, the frame-coupled camera pose - see ARKitFrameSeqMeta's hasPose)
// carried by each RTP packet's header extension and attaches it to the decoded
// buffer as ARKitFrameSeqMeta; update()'s appsink pull site reads that meta,
// builds an ARKitVideoFrameBundle directly from it, and maps the buffer as CUDA
// memory (GST_MAP_CUDA) to get a CUdeviceptr for the color-texture pipeline.
// (The separate pose UDP channel (basePort+2) and its ARKitPoseReceiver/
// ARKitFrameCorrelator plumbing were removed once pose started riding inside the
// video RTP stream's own header extension instead - see project history for the
// prior architecture. JBU depth/matte upsampling and the human-stencil pipeline
// were removed earlier still - too noisy in practice to be useful for
// compositing; pose tracking is the part that remains valuable.)
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

protected:
	bool openOnThread();

	// Called from update()'s appsink-pull site once per successfully-decoded CUDA
	// frame (ticket E3). Resizes the CUDA-GL color texture on dimension change and
	// runs the NV12->RGBA conversion kernel to write the display color texture.
	// Must run on the GL-context-owning thread (same as update() itself already
	// does - see CudaGLInterop.h's threading note).
	void updateColorTexture(GstBuffer* buffer, CUdeviceptr devicePtr, int width, int height);
	bool ensureTexturePipelineInitialized(int width, int height);

	// Live-tested finding (ticket E3): no CUDA context is actually current on this
	// thread by the time update() runs cuGraphicsGLRegisterImage/kernel launches
	// here - GST_MAP_CUDA's own context handling (if any) doesn't leave one
	// current for callers afterward (confirmed via a live
	// CUDA_ERROR_INVALID_CONTEXT failure). So this class creates and owns its own
	// CUcontext instead, independent of nvcodec's internal GstCudaContext - safe
	// because CUDA's Unified Virtual Addressing (the default on 64-bit
	// platforms/modern CUDA) makes device pointers allocated under one context on a
	// device valid to dereference from another context on that same device, within
	// the same process - which is exactly what's needed to read a GST_MAP_CUDA
	// pointer from our own context. Lazily created once; cuCtxSetCurrent() is
	// re-asserted defensively at the start of every updateColorTexture() call
	// since anything else that ran on the main thread in between (including
	// GStreamer's own internal context push/pop during gst_buffer_map) could have
	// changed what's current.
	bool ensureCudaContext();

	// Fans an already-built ARKitVideoFrameBundle out to listeners. Called from
	// update()'s appsink-pull site, on the main/GL thread - unlike before pose
	// moved into the video RTP stream, this no longer fires from a separate
	// receiver worker thread, so listeners that assumed a background-thread call
	// (e.g. for locking purposes) should be re-examined.
	void notifyFrameBundleReceived(const ARKitVideoFrameBundle& bundle);

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

	std::mutex m_listenersMutex;
	std::set<IARKitVideoDeviceListener*> m_listeners;

	bool m_bTexturePipelineInitialized= false;
	int m_textureWidth= 0;
	int m_textureHeight= 0;

	// This class's own CUDA context (see ensureCudaContext()'s comment for why) -
	// independent of whatever context nvcodec's pipeline owns internally.
	CUcontext m_cudaContext= nullptr;

	CudaGLColorTexture m_colorTexture;
	NV12ConversionKernel m_nv12Kernel;

	// Full-resolution packed-RGB "guide" buffer NV12ConversionKernel::convert()
	// writes alongside the RGBA display texture - a mandatory output parameter of
	// that kernel (originally consumed by the now-removed JBU depth upsampler as
	// its guide image). Nothing reads it anymore; kept only because the kernel
	// still requires a valid destination to write to. Sized
	// m_textureHeight * m_guideStrideBytes.
	CUdeviceptr m_guideRgbBuffer= 0;
	int m_guideStrideBytes= 0;
};
