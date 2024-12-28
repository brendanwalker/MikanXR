#pragma once

//-- includes -----
#include "DeviceView.h"
#include "VideoSourceInterface.h"
#include "OpenCVFwd.h"
#include "glm/ext/matrix_float4x4.hpp"

#include <memory>
#include <vector>

// -- constants -----

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

#define LEFT_PROJECTION_INDEX  0
#define RIGHT_PROJECTION_INDEX 1

#define MONO_PROJECTION_COUNT 1
#define STEREO_PROJECTION_COUNT 2

#define MAX_PROJECTION_COUNT 2
#define PRIMARY_PROJECTION_INDEX LEFT_PROJECTION_INDEX

// -- declarations -----
class VideoSourceView : public DeviceView, public IVideoSourceListener
{
public:
	VideoSourceView(const int device_id);
	~VideoSourceView();

	static IVideoSourceInterface* allocateVideoSourceInterface(const class DeviceEnumerator* enumerator);

	bool open(const class DeviceEnumerator* enumerator) override;
	void close() override;

	bool startVideoStream();
	bool getIsVideoStreaming() const;
	void stopVideoStream();

	IDeviceInterface* getDevice() const override { return m_device; }
	IVideoSourceInterface* getVideoSourceInterface() const { return static_cast<IVideoSourceInterface*>(getDevice()); }

	// Returns what type of tracker this tracker view represents
	eDeviceType getVideoSourceDeviceType() const;

	// Returns what type of driver this video source uses
	IVideoSourceInterface::eDriverType getVideoSourceDriverType() const;

	// Returns true if this is a stereo camera
	bool getIsStereoCamera() const;

	// Returns a friendly name of the video source
	std::string getFriendlyName() const;

	// Returns the full usb device path for the controller
	std::string getUSBDevicePath() const;

	void loadSettings();
	void saveSettings();

	bool getAvailableVideoModes(std::vector<std::string>& out_mode_names) const;
	const struct VideoModeConfig* getVideoMode() const;
	bool setVideoMode(const std::string& new_mode);

	double getFrameWidth() const;
	double getFrameHeight() const;
	double getFrameRate() const;

	bool getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint& outConstraint) const;
	int getVideoPropertyConstraintStep(const VideoPropertyType property_type, int defaultStep = 1) const;
	int getVideoPropertyConstraintMinValue(const VideoPropertyType property_type, int defaultMin = 0) const;
	int getVideoPropertyConstraintMaxValue(const VideoPropertyType property_type, int defaultMax = 255) const;

	int getVideoProperty(const VideoPropertyType property_type) const;
	void setVideoProperty(const VideoPropertyType property_type, int desired_value, bool save_setting);

	bool hasNewVideoFrameAvailable(VideoFrameSection section) const;
	int64_t readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer);

	void getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const;
	void setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics);

	MikanQuatd getCameraOffsetOrientation() const;
	MikanVector3d getCameraOffsetPosition() const;
	void setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& p);
	glm::mat4 getCameraPose(VRDeviceViewPtr attachedVRDevicePtr, bool bApplyVRDeviceOffset= true) const;
	glm::mat4 getCameraProjectionMatrix() const;
	glm::mat4 getCameraViewMatrix(VRDeviceViewPtr attachedVRDevicePtr) const;
	glm::mat4 getCameraViewProjectionMatrix(VRDeviceViewPtr attachedVRDevicePtr) const;

	void getPixelDimensions(float& outWidth, float& outHeight) const;
	void getFOV(float& outHFOV, float& outVFOV) const;
	void getZRange(float& outZNear, float& outZFar) const;

	//-- ITrackerListener
	virtual void notifyVideoFrameReceived(const unsigned char* raw_video_frame_buffer) override;

protected:
	void reallocateOpencvBufferState();
	bool allocateDeviceInterface(const class DeviceEnumerator* enumerator) override;
	void freeDeviceInterface() override;
	void recomputeCameraProjectionMatrix();

private:
	int64_t m_lastVideoFrameReadIndex;
	class OpenCVBufferState* m_opencv_buffer_state[MAX_PROJECTION_COUNT];
	IVideoSourceInterface* m_device;
	glm::mat4 m_projectionMatrix;
};
