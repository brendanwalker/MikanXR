//-- includes -----
#include "CameraMath.h"
#include "DeviceEnumerator.h"
#include "VideoSourceView.h"
#include "MathUtility.h"
#include "MathGLM.h"
#include "OpenCVVideoSource.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoDeviceEnumerator.h"
#include "VRDeviceView.h"
#include "WMFMonoVideoSource.h"
#include "WMFStereoVideoSource.h"

#include <memory>

#include <glm/ext/matrix_clip_space.hpp>

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#include <algorithm>
#include <atomic>
#include <mutex>

#include <easy/profiler.h>

//-- private methods -----
class OpenCVBufferState
{
public:
	OpenCVBufferState(IVideoSourceInterface* device, VideoFrameSection _section)
		: m_section(_section)
		, m_bgrBuffer(nullptr)
		, m_lastVideoFrameWriteIndex(0)
	{
		const VideoModeConfig* mode = device->getVideoMode();

		m_srcBufferWidth = mode->bufferPixelWidth;
		m_srcBufferHeight = mode->bufferPixelHeight;
		device->getVideoFrameDimensions(&m_frameWidth, &m_frameHeight, nullptr);

		m_bgrBuffer = new cv::Mat(m_frameHeight, m_frameWidth, CV_8UC3);
	}

	virtual ~OpenCVBufferState()
	{
		if (m_bgrBuffer != nullptr)
		{
			delete m_bgrBuffer;
		}
	}

	void writeVideoFrame(const unsigned char* video_buffer, bool bIsFlipped)
	{
		EASY_FUNCTION();

		std::lock_guard<std::mutex> bufferLock(m_bufferMutex);
		const cv::Mat videoBufferMat(m_srcBufferHeight, m_srcBufferWidth, CV_8UC3, const_cast<unsigned char*>(video_buffer));

		if (bIsFlipped)
		{
			cv::flip(videoBufferMat, *m_bgrBuffer, +1);
		}
		else
		{
			videoBufferMat.copyTo(*m_bgrBuffer);
		}

		// Atomically increment the frame index on the write thread
		m_lastVideoFrameWriteIndex++;
	}

	void writeStereoVideoFrameSection(const unsigned char* video_buffer, const cv::Rect& buffer_bounds, bool bIsFlipped)
	{
		EASY_FUNCTION();

		std::lock_guard<std::mutex> bufferLock(m_bufferMutex);
		const cv::Mat videoBufferMat(m_srcBufferHeight, m_srcBufferWidth, CV_8UC3, const_cast<unsigned char*>(video_buffer));

		if (bIsFlipped)
		{
			cv::flip(videoBufferMat(buffer_bounds), *m_bgrBuffer, +1);
		}
		else
		{
			videoBufferMat(buffer_bounds).copyTo(*m_bgrBuffer);
		}

		// Atomically increment the frame index on the write thread
		m_lastVideoFrameWriteIndex++;
	}

	int64_t getLastVideoFrameWriteIndex() const
	{
		return m_lastVideoFrameWriteIndex.load();
	}

	int64_t readVideoFrame(cv::Mat* outBGRBuffer, int64_t lastReadFrameIndex)
	{
		EASY_FUNCTION();
		int64_t lastVideoFrameWriteIndex= getLastVideoFrameWriteIndex();

		if (lastVideoFrameWriteIndex != lastReadFrameIndex)
		{
			std::lock_guard<std::mutex> bufferLock(m_bufferMutex);

			m_bgrBuffer->copyTo(*outBGRBuffer);
		}

		return lastVideoFrameWriteIndex;
	}

private:
	VideoFrameSection m_section;

	int m_srcBufferWidth;
	int m_srcBufferHeight;
	int m_frameWidth;
	int m_frameHeight;

	std::mutex m_bufferMutex;
	cv::Mat* m_bgrBuffer; // source video frame
	std::atomic_int64_t m_lastVideoFrameWriteIndex;
};

//-- public implementation -----
VideoSourceView::VideoSourceView(const int device_id)
	: DeviceView(device_id)
	, m_lastVideoFrameReadIndex(0)
	, m_device(nullptr)
	, m_projectionMatrix(glm::mat4(1.f))
{
	for (int i = 0; i < MAX_PROJECTION_COUNT; ++i)
	{
		m_opencv_buffer_state[i] = nullptr;
	}
}

VideoSourceView::~VideoSourceView()
{
	for (int i = 0; i < MAX_PROJECTION_COUNT; ++i)
	{
		if (m_opencv_buffer_state[i] != nullptr)
		{
			delete m_opencv_buffer_state[i];
		}
	}

	if (m_device != nullptr)
	{
		delete m_device;
	}
}

eDeviceType VideoSourceView::getVideoSourceDeviceType() const
{
	return m_device->getDeviceType();
}

IVideoSourceInterface::eDriverType VideoSourceView::getVideoSourceDriverType() const
{
	return m_device->getDriverType();
}

bool VideoSourceView::getIsStereoCamera() const
{
	return m_device->getIsStereoCamera();
}

std::string VideoSourceView::getFriendlyName() const
{
	return m_device->getFriendlyName();
}

std::string VideoSourceView::getUSBDevicePath() const
{
	return m_device->getUSBDevicePath();
}

bool VideoSourceView::open(const class DeviceEnumerator* enumerator)
{
	bool bSuccess = DeviceView::open(enumerator);

	if (bSuccess)
	{
		// Allocate the open cv buffers used for tracking filtering
		reallocateOpencvBufferState();

		// Recompute the projection matrix
		recomputeCameraProjectionMatrix();

		// Let any connected clients know that the video source opened
		MikanServer::getInstance()->publishVideoSourceOpenedEvent();
	}

	return bSuccess;
}

void VideoSourceView::close()
{
	DeviceView::close();

	for (int i = 0; i < MAX_PROJECTION_COUNT; ++i)
	{
		if (m_opencv_buffer_state[i] != nullptr)
		{
			delete m_opencv_buffer_state[i];
			m_opencv_buffer_state[i] = nullptr;
		}
	}

	// Let any connected clients know that the video source closed
	MikanServer::getInstance()->publishVideoSourceClosedEvent();
}

bool VideoSourceView::startVideoStream()
{
	if (m_device != nullptr)
	{
		if (!m_device->getIsVideoStreaming())
		{
			return m_device->startVideoStream();
		}
		else
		{
			return true;
		}
	}

	return false;
}

bool VideoSourceView::getIsVideoStreaming() const
{
	return (m_device != nullptr && m_device->getIsVideoStreaming());
}

void VideoSourceView::stopVideoStream()
{
	if (getIsVideoStreaming())
	{
		m_device->stopVideoStream();
	}
}

void VideoSourceView::notifyVideoFrameReceived(const unsigned char* raw_video_frame_buffer)
{
	const bool is_frame_flipped = m_device->getIsFrameMirrored();
	const bool is_buffer_flipped = m_device->getIsBufferMirrored();

	if (m_device == nullptr)
	{
		return;
	}

	// Fetch the latest video buffer frame from the device
	if (m_device->getIsStereoCamera())
	{
		const VideoModeConfig* mode_config = m_device->getVideoMode();
		const auto& stereoIntrinsics= mode_config->intrinsics.getStereoIntrinsics();
		const int section_width = (int)stereoIntrinsics.pixel_width;
		const int section_height = (int)stereoIntrinsics.pixel_height;

		cv::Rect left_bounds;
		cv::Rect right_bounds;

		if (mode_config->frameSections.size() >= 2)
		{
			const VideoFrameSectionInfo& left_section = mode_config->frameSections[0];
			const VideoFrameSectionInfo& right_section = mode_config->frameSections[1];

			left_bounds = cv::Rect(left_section.x, left_section.y, section_width, section_height);
			right_bounds = cv::Rect(right_section.x, right_section.y, section_width, section_height);
		}
		else
		{
			left_bounds = cv::Rect(0, 0, section_width, section_height);
			right_bounds = cv::Rect(section_width, 0, section_width, section_height);
		}

		// Cache the left raw video frame
		if (m_opencv_buffer_state[(int)VideoFrameSection::Left] != nullptr)
		{
			m_opencv_buffer_state[(int)VideoFrameSection::Left]->writeStereoVideoFrameSection(
				raw_video_frame_buffer,
				is_buffer_flipped ? right_bounds : left_bounds,
				is_frame_flipped);
		}

		// Cache the right raw video frame
		if (m_opencv_buffer_state[(int)VideoFrameSection::Right] != nullptr)
		{
			m_opencv_buffer_state[(int)VideoFrameSection::Right]->writeStereoVideoFrameSection(
				raw_video_frame_buffer,
				is_buffer_flipped ? left_bounds : right_bounds,
				is_frame_flipped);
		}
	}
	else
	{
		// Cache the raw video frame
		if (m_opencv_buffer_state[(int)VideoFrameSection::Primary] != nullptr)
		{
			m_opencv_buffer_state[(int)VideoFrameSection::Primary]->writeVideoFrame(
				raw_video_frame_buffer, is_frame_flipped);
		}
	}
}

void VideoSourceView::reallocateOpencvBufferState()
{
	// Delete any existing opencv buffers
	for (int i = 0; i < MAX_PROJECTION_COUNT; ++i)
	{
		if (m_opencv_buffer_state[i] != nullptr)
		{
			delete m_opencv_buffer_state[i];
			m_opencv_buffer_state[i] = nullptr;
		}
	}

	// Allocate the OpenCV scratch buffers used for finding tracking blobs
	if (m_device->getIsStereoCamera())
	{
		m_opencv_buffer_state[(int)VideoFrameSection::Left] =
			new OpenCVBufferState(m_device, VideoFrameSection::Left);
		m_opencv_buffer_state[(int)VideoFrameSection::Right] =
			new OpenCVBufferState(m_device, VideoFrameSection::Right);
	}
	else
	{
		m_opencv_buffer_state[(int)VideoFrameSection::Primary] =
			new OpenCVBufferState(m_device, VideoFrameSection::Primary);
	}
}

bool VideoSourceView::allocateDeviceInterface(const class DeviceEnumerator* enumerator)
{
	m_device = VideoSourceView::allocateVideoSourceInterface(enumerator);

	if (m_device != nullptr)
	{
		m_device->setVideoSourceListener(this);
	}

	return m_device != nullptr;
}

IVideoSourceInterface* VideoSourceView::allocateVideoSourceInterface(const DeviceEnumerator* enumerator)
{
	const VideoDeviceEnumerator* videoEnumerator = static_cast<const VideoDeviceEnumerator*>(enumerator);
	IVideoSourceInterface* tracker_interface = nullptr;

	switch (videoEnumerator->getVideoApi())
	{
	case eVideoDeviceApi::OPENCV:
		{
			tracker_interface = new OpenCVVideoSource();
		} break;
#ifdef _WIN32
	case eVideoDeviceApi::WMF:
		{
			switch (enumerator->getDeviceType())
			{
			case eDeviceType::MonoVideoSource:
				{
					tracker_interface = new WMFMonoVideoSource();
				} break;
			case eDeviceType::StereoVideoSource:
				{
					tracker_interface = new WMFStereoVideoSource();
				} break;
			default:
				break;
			}
		} break;
#endif
	default:
		break;
	}

	return tracker_interface;
}

void VideoSourceView::freeDeviceInterface()
{
	if (m_device != nullptr)
	{
		delete m_device;  // Deleting abstract object should be OK because
		// this (ServerDeviceView) is abstract as well.
		// All non-abstract children will have non-abstract types
		// for m_device.
		m_device = nullptr;
	}
}

void VideoSourceView::loadSettings()
{
	m_device->loadSettings();
}

void VideoSourceView::saveSettings()
{
	m_device->saveSettings();
}

bool VideoSourceView::getAvailableVideoModes(std::vector<std::string>& out_mode_names) const
{
	return m_device->getAvailableTrackerModes(out_mode_names);
}

const VideoModeConfig* VideoSourceView::getVideoMode() const
{
	return m_device->getVideoMode();
}

bool VideoSourceView::setVideoMode(const std::string& new_mode)
{
	if (m_device != nullptr && m_device->getIsOpen())
	{
		bool bWasStreaming = getIsVideoStreaming();
		bool bUpdatedMode= false;

		if (bWasStreaming)
		{
			m_device->stopVideoStream();
		}

		if (m_device->setVideoMode(new_mode))
		{
			// Resize the opencv buffers
			reallocateOpencvBufferState();

			// Recompute the projection matrix
			recomputeCameraProjectionMatrix();

			// Let any connected clients know that the video mode changed
			MikanServer::getInstance()->publishVideoSourceModeChangedEvent();

			bUpdatedMode= true;
		}

		if (bWasStreaming)
		{
			m_device->startVideoStream();
		}

		return bUpdatedMode;
	}

	return false;
}

double VideoSourceView::getFrameWidth() const
{
	return m_device->getFrameWidth();
}

double VideoSourceView::getFrameHeight() const
{
	return m_device->getFrameHeight();
}

double VideoSourceView::getFrameRate() const
{
	return m_device->getFrameRate();
}

bool VideoSourceView::getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint& outConstraint) const
{
	return m_device->getVideoPropertyConstraint(property_type, outConstraint);
}

int VideoSourceView::getVideoPropertyConstraintStep(const VideoPropertyType property_type, int defaultStep) const
{
	VideoPropertyConstraint constraint;
	if (getVideoPropertyConstraint(property_type, constraint))
	{
		return constraint.stepping_delta;
	}
	else
	{
		return defaultStep;
	}
}

int VideoSourceView::getVideoPropertyConstraintMinValue(const VideoPropertyType property_type, int defaultMin) const
{
	VideoPropertyConstraint constraint;
	if (getVideoPropertyConstraint(property_type, constraint))
	{
		return constraint.min_value;
	}
	else
	{
		return defaultMin;
	}
}

int VideoSourceView::getVideoPropertyConstraintMaxValue(const VideoPropertyType property_type, int defaultMax) const
{
	VideoPropertyConstraint constraint;
	if (getVideoPropertyConstraint(property_type, constraint))
	{
		return constraint.max_value;
	}
	else
	{
		return defaultMax;
	}
}

int VideoSourceView::getVideoProperty(const VideoPropertyType property_type) const
{
	return m_device->getVideoProperty(property_type);
}

void VideoSourceView::setVideoProperty(const VideoPropertyType property_type, int desired_value, bool save_setting)
{
	m_device->setVideoProperty(property_type, desired_value, save_setting);
}

void VideoSourceView::getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const
{
	m_device->getCameraIntrinsics(out_camera_intrinsics);
}

void VideoSourceView::setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics)
{
	m_device->setCameraIntrinsics(camera_intrinsics);
	recomputeCameraProjectionMatrix();

	// Let any connected clients know that the video source intrinsics changed
	MikanServer::getInstance()->publishVideoSourceIntrinsicsChangedEvent();
}

MikanQuatd VideoSourceView::getCameraOffsetOrientation() const
{
	return m_device->getCameraOffsetOrientation();
}

MikanVector3d VideoSourceView::getCameraOffsetPosition() const
{
	return m_device->getCameraOffsetPosition();
}

void VideoSourceView::setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& p)
{
	m_device->setCameraPoseOffset(q, p);

	// Let any connected clients know that the video source attachment settings changed
	MikanServer::getInstance()->publishVideoSourceAttachmentChangedEvent();
}

glm::mat4 VideoSourceView::getCameraPose(
	VRDeviceViewPtr attachedVRDevicePtr,
	bool bApplyVRDeviceOffset) const
{
	// Get the pose of the VR device we want to compute the camera pose from
	const glm::mat4 vrDevicePose = attachedVRDevicePtr->getDefaultComponentPose(bApplyVRDeviceOffset);

	// Get the offset from the puck to the camera
	const glm::vec3 cameraOffsetPos = MikanVector3d_to_glm_dvec3(getCameraOffsetPosition());
	const glm::quat cameraOffsetQuat = MikanQuatd_to_glm_dquat(getCameraOffsetOrientation());
	const glm::mat4 cameraOffsetXform =
		glm::translate(glm::mat4(1.0), cameraOffsetPos) *
		glm::mat4_cast(cameraOffsetQuat);

	// Update the transform of the camera so that vr models align over the tracking puck
	const glm::mat4 cameraPose = vrDevicePose * cameraOffsetXform;

	return cameraPose;
}

glm::mat4 VideoSourceView::getCameraProjectionMatrix() const 
{
	return m_projectionMatrix;
}

void VideoSourceView::recomputeCameraProjectionMatrix()
{
	MikanVideoSourceIntrinsics camera_intrinsics;
	m_device->getCameraIntrinsics(camera_intrinsics);

	float videoSourcePixelWidth = 0.f;
	float videoSourcePixelHeight = 0.f;
	float vFov = 0.f;
	float zNear = 0.f;
	float zFar = 0.f;

	switch (camera_intrinsics.intrinsics_type)
	{
	case MONO_CAMERA_INTRINSICS:
		{
			const MikanMonoIntrinsics& monoIntrinsics = camera_intrinsics.getMonoIntrinsics();

			videoSourcePixelWidth = (float)monoIntrinsics.pixel_width;
			videoSourcePixelHeight = (float)monoIntrinsics.pixel_height;
			vFov = (float)monoIntrinsics.vfov;
			zNear = (float)monoIntrinsics.znear;
			zFar = (float)monoIntrinsics.zfar;
		} break;
	case STEREO_CAMERA_INTRINSICS:
		{
			const MikanStereoIntrinsics& stereoIntrinsics = camera_intrinsics.getStereoIntrinsics();

			videoSourcePixelWidth = (float)stereoIntrinsics.pixel_width;
			videoSourcePixelHeight = (float)stereoIntrinsics.pixel_height;
			vFov = (float)stereoIntrinsics.vfov;
			zNear = (float)stereoIntrinsics.znear;
			zFar = (float)stereoIntrinsics.zfar;
		} break;
	}

	if (videoSourcePixelWidth > 0 && videoSourcePixelHeight > 0 &&
		vFov > 0 && zNear > 0 && zFar > 0)
	{
		m_projectionMatrix =
			glm::perspective(
				degrees_to_radians(vFov),
				videoSourcePixelWidth / videoSourcePixelHeight,
				zNear,
				zFar);
	}
}

glm::mat4 VideoSourceView::getCameraViewMatrix(VRDeviceViewPtr attachedVRDevicePtr) const
{
	const glm::mat4 cameraPose = VideoSourceView::getCameraPose(attachedVRDevicePtr);
	const glm::mat4 viewMatrix = computeGLMCameraViewMatrix(cameraPose);

	return viewMatrix;
}

glm::mat4 VideoSourceView::getCameraViewProjectionMatrix(VRDeviceViewPtr attachedVRDevicePtr) const
{
	const glm::mat4 viewMatrix = getCameraViewMatrix(attachedVRDevicePtr);
	const glm::mat4 vpMatrix = m_projectionMatrix * viewMatrix;

	return vpMatrix;
}

void VideoSourceView::getPixelDimensions(float& outWidth, float& outHeight) const
{
	int pixelWidth, pixelHeight;

	m_device->getVideoFrameDimensions(&pixelWidth, &pixelHeight, nullptr);

	outWidth = static_cast<float>(pixelWidth);
	outHeight = static_cast<float>(pixelHeight);
}

void VideoSourceView::getFOV(float& outHFOV, float& outVFOV) const
{
	m_device->getFOV(outHFOV, outVFOV);
}

void VideoSourceView::getZRange(float& outZNear, float& outZFar) const
{
	m_device->getZRange(outZNear, outZFar);
}

bool VideoSourceView::hasNewVideoFrameAvailable(VideoFrameSection section) const
{
	int64_t lastFrameWriteIndex = 0;

	if (m_device->getIsStereoCamera())
	{
		if ((section == VideoFrameSection::Left || section == VideoFrameSection::Right) &&
			m_opencv_buffer_state[(int)section] != nullptr)
		{
			lastFrameWriteIndex = m_opencv_buffer_state[(int)section]->getLastVideoFrameWriteIndex();
		}
	}
	else
	{
		if (section == VideoFrameSection::Primary &&
			m_opencv_buffer_state[(int)VideoFrameSection::Primary] != nullptr)
		{
			lastFrameWriteIndex = m_opencv_buffer_state[(int)VideoFrameSection::Primary]->getLastVideoFrameWriteIndex();
		}
	}

	return lastFrameWriteIndex != m_lastVideoFrameReadIndex;
}

int64_t VideoSourceView::readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer)
{
	EASY_FUNCTION();

	if (m_device->getIsStereoCamera())
	{
		if ((section == VideoFrameSection::Left || section == VideoFrameSection::Right) &&
			m_opencv_buffer_state[(int)section] != nullptr)
		{
			m_lastVideoFrameReadIndex = 
				m_opencv_buffer_state[(int)section]->readVideoFrame(
					outBuffer, 
					m_lastVideoFrameReadIndex);
		}
	}
	else
	{
		if (section == VideoFrameSection::Primary &&
			m_opencv_buffer_state[(int)VideoFrameSection::Primary] != nullptr)
		{
			m_lastVideoFrameReadIndex = 
				m_opencv_buffer_state[(int)VideoFrameSection::Primary]->readVideoFrame(
					outBuffer, 
					m_lastVideoFrameReadIndex);
		}
	}

	return m_lastVideoFrameReadIndex;
}