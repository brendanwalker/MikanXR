#pragma once

// -- includes -----
#include "IVideoDevice.h"

#include <cstdint>
#include <memory>

// -- definitions -----

struct ARKitVideoConnectionSettings
{
	uint16_t basePort= 0; // video RTP on basePort+0, depth on basePort+1, pose on basePort+2
	bool depthStreamingEnabled= true;
};

// Depth+confidence planes for one frame. Pointers are only valid for the duration
// of the notifyFrameBundleReceived callback - copy out anything needed afterward.
// Raw-pointer-and-count rather than std::vector/std::optional, matching this
// header's cross-DLL-boundary convention (see NetworkVideoFrameBuffer in
// INetworkVideoDevice.h).
struct ARKitDepthFrameBuffer
{
	int width= 0;                       // pixels
	int height= 0;                      // pixels
	const uint16_t* depthMM= nullptr;   // width*height samples, millimeters, 0=invalid
	const uint8_t* confidence= nullptr; // width*height samples, 0/1/2
};

struct ARKitPoseFrameBuffer
{
	float transform[16]= {}; // row-major 4x4, camera-to-world
	float fx= 0.f;
	float fy= 0.f;
	float cx= 0.f;
	float cy= 0.f;
	float imageWidth= 0.f;
	float imageHeight= 0.f;
};

// A frame-correlated bundle of video + (optional) depth + (optional) pose for the
// same frameSeq. hasVideo/hasDepth/hasPose unset represents a component that never
// arrived in time - not necessarily an error; e.g. pose alone is still useful to
// drive camera tracking even without video/depth (see IFrameCoupledPoseProvider,
// ticket E4). This is the public, DLL-boundary-safe equivalent of the
// MikanARKitVideo plugin's internal ARKitFrameCorrelator::ARKitFrameBundle type -
// the two intentionally aren't shared directly, since MikanCoreApp (built before
// Plugins) can't depend on a specific plugin's private headers.
struct ARKitVideoFrameBundle
{
	uint32_t frameSeq= 0;
	uint64_t timestampUs= 0;

	bool hasVideo= false;
	const uint8_t* videoData= nullptr; // decoded frame buffer, valid only during the callback
	int videoWidth= 0;
	int videoHeight= 0;

	bool hasDepth= false;
	ARKitDepthFrameBuffer depth;

	bool hasPose= false;
	ARKitPoseFrameBuffer pose;
};

class IARKitVideoDeviceListener
{
public:
	// Called when the async open has completed successfully
	virtual void notifyDeviceOpened(const class IARKitVideoDevice* device) {}

	// Called when the video source has been disconnected
	virtual void notifyDeviceClosed(const class IARKitVideoDevice* device)= 0;

	// Called when a correlated video/depth/pose bundle is ready for a frameSeq
	virtual void notifyFrameBundleReceived(const ARKitVideoFrameBundle& bundle)= 0;
};

// ARKitVideoDevice interface
class IARKitVideoDevice : public IVideoDevice
{
public:
	IARKitVideoDevice()= default;
	virtual ~IARKitVideoDevice() {}

	virtual void update(float deltaSeconds)= 0;

	// -- Device Listener
	virtual void addListener(IARKitVideoDeviceListener* listener)= 0;
	virtual void removeListener(IARKitVideoDeviceListener* listener)= 0;

	// -- Stream Properties
	virtual const char* getDevicePath() const= 0;
	virtual const char* getFriendlyName() const= 0;

	// -- Device Activation
	virtual eVideoOpeningStatus getVideoOpeningStatus() const= 0;
	virtual eVideoOpeningStatus open()= 0;
	virtual void close()= 0;

	// -- Video Streaming
	virtual eVideoStreamingStatus startVideoStream()= 0;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const= 0;
	virtual void stopVideoStream()= 0;

	// -- Zero-copy CUDA-GL texture access (ticket E3) --
	// Returns the GL texture id (name) of the most recently decoded/upsampled
	// color/depth frame, or 0 if none has arrived yet. MikanCoreApp (this header's
	// library) is a lower layer than MikanRenderer - it can't depend on
	// IMkTexturePtr - so this crosses the boundary as a raw GL texture id;
	// IARKitVideoDeviceListener's caller (Editor-side code, which does depend on
	// MikanRenderer) is expected to wrap it via
	// IMkExternalTexture::setExternalPlatformTexture(&glId) (see
	// ARKitVideoSourceComponent::getDirectColorTexture/getDirectDepthTexture). Both
	// textures are written in place every frame (CUDA WRITE_DISCARD - see
	// CudaGLInterop.h), so the same non-zero id remains valid to keep reusing
	// across frames; only a resize/reopen changes it. Must be called from the
	// GL-context-owning thread, same as every other CUDA-GL interop call in this
	// pipeline.
	virtual uint32_t getColorTextureGlId() const= 0;
	virtual uint32_t getDepthTextureGlId() const= 0;
};

using IARKitVideoDevicePtr= std::shared_ptr<IARKitVideoDevice>;
using IARKitVideoDeviceConstPtr= std::shared_ptr<const IARKitVideoDevice>;
using IARKitVideoDeviceWeakPtr= std::weak_ptr<IARKitVideoDevice>;
