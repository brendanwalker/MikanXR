#pragma once

#include "glm/ext/matrix_float4x4.hpp"

#include <cstdint>

struct MikanVideoSourceIntrinsics;

// Implemented by video sources that deliver camera pose coupled to each video
// frame (currently just ARKitVideoSourceComponent - an iPhone's ARKit session
// reports pose alongside each captured frame, riding inside the video RTP
// stream's own header extension - see ARKitRTPHeaderExtension.h), as opposed to
// every other video source type, whose camera pose comes from polling a separate SteamVR-style
// tracked puck once per engine tick (CameraComponent::updateAperturePoseFromTrackingMount)
// independent of when a video frame actually arrives (ticket E4).
//
// Deliberately NOT a MikanComponent - a stateless mixin interface, the same shape
// as IARKitVideoDeviceListener (which ARKitVideoSourceComponent already multiply-
// inherits alongside VideoSourceComponent), so implementing it doesn't introduce
// any new component/entity plumbing.
class IFrameCoupledPoseProvider
{
public:
	virtual ~IFrameCoupledPoseProvider() {}

	// Returns the most recently received frame-coupled pose/intrinsics, or false if
	// none have arrived yet (e.g. the device hasn't opened, or no pose packet has
	// been received since). outFrameSeq lets the caller correlate this pose with
	// the video frame it corresponds to (see VideoSourceComponent::getDirectFrameIndex()).
	// outTransform is camera-to-world (matches CameraComponent::setRelativeTransform's
	// expected space).
	virtual bool getLatestFrameCoupledPose(glm::mat4& outTransform, MikanVideoSourceIntrinsics& outIntrinsics,
										   uint32_t& outFrameSeq) const= 0;

	// The transform taking this source's own world space into stage space,
	// applied to every incoming pose. A frame-coupled camera moves, so it cannot
	// be placed with a static CameraComponent::setRelativeTransform the way a
	// tripod-mounted camera is; alignment instead solves for this offset once and
	// leaves the per-frame poses to flow through it (AppStage_AlignCameraByOriginMarker).
	//
	// Identity means poses are passed through in the source's own world space.
	virtual void setPoseOffset(const glm::mat4& worldToStageXform)= 0;
	virtual glm::mat4 getPoseOffset() const= 0;
	virtual bool hasPoseOffset() const= 0;
};
