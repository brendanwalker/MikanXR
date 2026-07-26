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

// Joint Bilateral Upsampling tuning params - the public, DLL-boundary-safe mirror of
// the MikanARKitVideo plugin's private JBUParams (Cuda/JBUKernel.h), same rationale
// as ARKitVideoFrameBundle mirroring ARKitFrameCorrelator::ARKitFrameBundle: this
// header (MikanCoreApp) can't depend on a specific plugin's private headers. Field
// defaults match JBUParams' own ticket-D5-tuned defaults. radius/sigmaSpatial/
// sigmaColor are runtime-tunable (see setJBUParams()) - sigmaSpatial/sigmaColor of
// exactly 0 would divide-by-zero in the kernel's Gaussian weighting, so callers
// should keep them comfortably above 0 (ARKitVideoSourceDefinition's setters clamp
// to a safe minimum).
// What the device renders into its R32F depth-preview texture each frame. jbu_upsampled
// is the normal path (and what the compositor consumes); the other two are transient
// debug views for the video-settings window that nearest-upsample the raw low-res planes
// so you can see what's actually arriving (see IARKitVideoDevice::setDepthPreviewMode).
enum class eARKitDepthPreviewMode : int
{
	jbu_upsampled= 0,
	raw_depth_nearest,
	matte_nearest,         // raw nearest-upsampled 0/1 stencil
	refined_matte_nearest, // guided stencil-JBU (crisp alpha), scaled for the depth-preview shader
};

struct ARKitJBUParams
{
	int radius= 32;
	float sigmaSpatial= 16.0f;
	float sigmaColor= 15.0f;
	float confWeightLow= 0.0f;
	float confWeightMedium= 0.5f;

	// Person-segmentation silhouette gating (mirror of JBUParams' seg fields). When
	// segGatingEnabled, the device gates the upsample against the streamed person matte
	// so depth can't bleed across a person's silhouette; segEdgeStrength is the
	// cross-boundary tap weight (1.0 == no gating, 0.0 == hard crisp edge, clamped to
	// [0,1] by the definition layer). No effect on devices that never receive a matte.
	bool segGatingEnabled= false;
	float segEdgeStrength= 0.0f;
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

	// -- Human-stencil (person matte) textures --
	// Full-resolution R32F person alpha in [0,1], exposed as selectable compositor inputs
	// (see eVideoTextureSource::human_stencil_texture / human_stencil_raw_texture). Refined
	// is the guided-upsampled matte (crisp, edge snapped to the RGB guide - normally what
	// you composite with); Raw is the nearest-upsampled 0/1 stencil (blocky but not
	// color-dependent - a debugging ground truth when the guided upsample misbehaves in
	// tricky lighting). Same GL-id-crossing-the-boundary contract as the depth texture;
	// 0 until the video pipeline initializes, then all-zero alpha ("no person") until a
	// matte arrives. Must be called from the GL-context-owning thread.
	virtual uint32_t getHumanStencilRefinedTextureGlId() const= 0;
	virtual uint32_t getHumanStencilRawTextureGlId() const= 0;

	// -- JBU Tuning --
	// Updates the live depth-upsample pipeline's Joint Bilateral Upsampling params.
	// Safe to call at any time (including while streaming) - takes effect on the next
	// frame's upsample, no reopen/restart needed. Implementations must make this
	// safe to call from a different thread than update() (see
	// MikanARKitVideoDevice's own listener/property threading notes elsewhere in
	// this codebase for why that caution is warranted here).
	virtual void setJBUParams(const ARKitJBUParams& params)= 0;

	// -- Depth preview mode (transient debug view) --
	// Selects what the device renders into its depth-preview texture: the normal
	// JBU-upsampled depth, or a nearest-upsampled view of the raw low-res depth or person
	// matte. Safe to call from any thread at any time; takes effect on the next frame.
	// NOTE: this repurposes the single shared depth texture, so while a debug view is
	// selected the compositor sees that view too - callers should restore jbu_upsampled
	// when leaving the preview UI.
	virtual void setDepthPreviewMode(eARKitDepthPreviewMode mode)= 0;
};

using IARKitVideoDevicePtr= std::shared_ptr<IARKitVideoDevice>;
using IARKitVideoDeviceConstPtr= std::shared_ptr<const IARKitVideoDevice>;
using IARKitVideoDeviceWeakPtr= std::weak_ptr<IARKitVideoDevice>;
