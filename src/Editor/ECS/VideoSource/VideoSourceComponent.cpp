#include "CameraMath.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "OpenCVVideoFrameBuffer.h"
#include "VideoSourceComponent.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <easy/profiler.h>

// -- VideoSourceDefinition -----
const std::string VideoSourceDefinition::k_videoSourceIdPropertyId = "video_source_id";
const std::string VideoSourceDefinition::k_videoSourceIntrinsicsPropertyId= "video_source_intrinsics";
const std::string VideoSourceDefinition::k_hasValidIntrinsicsPropertyId = "are_intrinsics_valid";
const std::string VideoSourceDefinition::k_isFrameMirroredPropertyId = "is_frame_mirrored";
const std::string VideoSourceDefinition::k_isBufferMirroredPropertyId = "is_buffer_mirrored";
const std::string VideoSourceDefinition::k_videoFrameQueueSizePropertyId = "video_frame_queue_size";

VideoSourceDefinition::VideoSourceDefinition()
	: MikanComponentDefinition()
	, m_videoSourceId(INVALID_MIKAN_ID)
	, m_intrinsics()
{}

VideoSourceDefinition::VideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const std::string& videoSourceName)
	: MikanComponentDefinition(videoSourceId, videoSourceName)
	, m_videoSourceId(videoSourceId)
{}

VideoSourceDefinition::VideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const std::string& videoSourceName,
	const MikanVideoSourceIntrinsics& intrinsics)
	: MikanComponentDefinition(videoSourceId, videoSourceName)
	, m_videoSourceId(videoSourceId)
	, m_intrinsics(intrinsics)
{}

configuru::Config VideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt[VideoSourceDefinition::k_videoSourceIdPropertyId] = m_videoSourceId;
	pt[VideoSourceDefinition::k_isFrameMirroredPropertyId] = m_bIsFrameMirrored;
	pt[VideoSourceDefinition::k_isBufferMirroredPropertyId] = m_bIsBufferMirrored;
	pt[VideoSourceDefinition::k_videoFrameQueueSizePropertyId] = m_videoFrameQueueSize;

	switch (m_intrinsics.intrinsics_type)
	{
		case MONO_CAMERA_INTRINSICS:
			pt["intrinsics_type"] = std::string("mono");
			CommonConfig::writeMonoTrackerIntrinsics(pt, m_intrinsics.getMonoIntrinsics());
			break;
		case STEREO_CAMERA_INTRINSICS:
			pt["intrinsics_type"] = std::string("stereo");
			CommonConfig::writeStereoTrackerIntrinsics(pt, m_intrinsics.getStereoIntrinsics());
			break;
	}

	return pt;
}

void VideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_videoSourceId = pt.get_or<MikanVideoSourceID>("video_source_id", m_videoSourceId);
	m_bIsFrameMirrored = pt.get_or<bool>("is_frame_mirrored", false);
	m_bIsBufferMirrored = pt.get_or<bool>("is_buffer_mirrored", false);
	m_videoFrameQueueSize = pt.get_or<int>("video_frame_queue_size", 10);

	std::string intrinsics_type = pt.get_or<std::string>("intrinsics_type", "");
	if (intrinsics_type == "mono")
	{
		MikanMonoIntrinsics monoIntrinsics = {};
		CommonConfig::readMonoTrackerIntrinsics(pt, monoIntrinsics);

		m_intrinsics.makeMonoIntrinsics() = monoIntrinsics;
	}
	else if (intrinsics_type == "stereo")
	{
		MikanStereoIntrinsics stereoIntrinsics = {};
		CommonConfig::readStereoTrackerIntrinsics(pt, stereoIntrinsics);
		m_intrinsics.intrinsics_type = STEREO_CAMERA_INTRINSICS;

		m_intrinsics.makeStereoIntrinsics() = stereoIntrinsics;
	}
	else
	{
		m_intrinsics = MikanVideoSourceIntrinsics();
	}
}

void VideoSourceDefinition::setIsFrameMirrored(bool isFrameMirrored)
{
	m_bIsFrameMirrored = isFrameMirrored;
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_isFrameMirroredPropertyId));
}

void VideoSourceDefinition::setIsBufferMirrored(bool isBufferMirrored)
{
	m_bIsBufferMirrored = isBufferMirrored;
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_isBufferMirroredPropertyId));
}

void VideoSourceDefinition::setVideoFrameQueueSize(int videoFrameQueueSize)
{
	if (m_videoFrameQueueSize != videoFrameQueueSize)
	{
		m_videoFrameQueueSize = videoFrameQueueSize;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_videoFrameQueueSizePropertyId));
	}
}

void VideoSourceDefinition::setCameraIntrinsics(
	const MikanVideoSourceIntrinsics& cameraIntrinsics)
{
	m_intrinsics = cameraIntrinsics;
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_videoSourceIntrinsicsPropertyId));
}

// -- VideoSourceComponent -----
VideoSourceComponent::VideoSourceComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, m_lastVideoFrameReadIndex(0)
	, m_projectionMatrix(glm::mat4(1.f))
{
	for (int i = 0; i < MAX_PROJECTION_COUNT; ++i)
	{
		m_opencv_buffer_state[i] = nullptr;
	}
}

void VideoSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	auto videoSourceDefinitionPtr = std::static_pointer_cast<VideoSourceDefinition>(definition);
}

MikanVideoSourceID VideoSourceComponent::getVideoSourceId() const
{
	return getVideoSourceDefinition()->getVideoSourceId();
}

bool VideoSourceComponent::getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const
{
	return false;
}

bool VideoSourceComponent::hasNewVideoFrameAvailable(VideoFrameSection section) const
{
	MikanVideoSourceIntrinsics intrinsics;
	if (getCameraIntrinsics(intrinsics))
	{
		int64_t lastFrameWriteIndex = 0;

		if (intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
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

	return false;
}

int64_t VideoSourceComponent::readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer)
{
	EASY_FUNCTION();

	MikanVideoSourceIntrinsics intrinsics;
	if (getCameraIntrinsics(intrinsics))
	{
		if (intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
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

	return 0;
}

bool VideoSourceComponent::getVideoModeName(std::string& outVideoModeName) const
{
	return false;
}

bool VideoSourceComponent::getFrameRate(float& outFrameRate) const
{
	return false;
}

bool VideoSourceComponent::getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const
{
	if (!getVideoSourceDefinition()->hasCameraIntrinsics())
	{
		out_camera_intrinsics = MikanVideoSourceIntrinsics();

		return true;
	}

	return false;
}

bool VideoSourceComponent::setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics)
{
	getVideoSourceDefinition()->setCameraIntrinsics(camera_intrinsics);
	recomputeCameraProjectionMatrix();

	return true;
}

glm::mat4 VideoSourceComponent::getProjectionMatrix() const
{
	return m_projectionMatrix;
}

bool VideoSourceComponent::isVideoSettingSupported(const eVideoSettingType property_type) const
{
	return false;
}

bool VideoSourceComponent::getVideoSettingConstraint(const eVideoSettingType property_type, VideoSettingConstraint& outConstraint) const
{
	return false;
}

void VideoSourceComponent::setVideoSetting(const eVideoSettingType property_type, int desired_value)
{
}

int VideoSourceComponent::getVideoSetting(const eVideoSettingType property_type) const
{
	return -1;
}

bool VideoSourceComponent::hasAllocatedOpencvBufferState() const
{
	return m_opencv_buffer_state[0] != nullptr;
}

bool VideoSourceComponent::reallocateOpencvBufferState()
{
	releaseOpencvBufferState();

	int videoPixelWidth, videoPixelHeight;
	if (!getVideoPixelDimensions(videoPixelWidth, videoPixelHeight))
		return false;

	MikanVideoSourceIntrinsics intrinsics;
	if (!getCameraIntrinsics(intrinsics))
		return false;

	// Allocate the OpenCV scratch buffers used for finding tracking blobs
	if (intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS)
	{
		const MikanStereoIntrinsics& stereoIntrinsics = intrinsics.getStereoIntrinsics();

		m_opencv_buffer_state[(int)VideoFrameSection::Left] =
			new OpenCVVideoFrameBuffer(
				videoPixelWidth, videoPixelHeight,
				stereoIntrinsics.pixel_width, stereoIntrinsics.pixel_width,
				VideoFrameSection::Left);
		m_opencv_buffer_state[(int)VideoFrameSection::Right] =
			new OpenCVVideoFrameBuffer(
				videoPixelWidth, videoPixelHeight,
				stereoIntrinsics.pixel_width, stereoIntrinsics.pixel_width,
				VideoFrameSection::Right);
	}
	else if (intrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS)
	{
		const MikanMonoIntrinsics& monoIntrinsics = intrinsics.getMonoIntrinsics();

		m_opencv_buffer_state[(int)VideoFrameSection::Primary] =
			new OpenCVVideoFrameBuffer(
				videoPixelWidth, videoPixelHeight,
				monoIntrinsics.pixel_width, monoIntrinsics.pixel_width,
				VideoFrameSection::Primary);
	}
	else
	{
		m_opencv_buffer_state[(int)VideoFrameSection::Primary] =
			new OpenCVVideoFrameBuffer(
				videoPixelWidth, videoPixelHeight,
				videoPixelWidth, videoPixelHeight, // Frame Size == Buffer Size
				VideoFrameSection::Primary);
	}

	return true;
}

void VideoSourceComponent::releaseOpencvBufferState()
{
	// Delete any existing OpenCV buffers
	for (int i = 0; i < MAX_PROJECTION_COUNT; ++i)
	{
		if (m_opencv_buffer_state[i] != nullptr)
		{
			delete m_opencv_buffer_state[i];
			m_opencv_buffer_state[i] = nullptr;
		}
	}
}

void VideoSourceComponent::recomputeCameraProjectionMatrix()
{
	MikanVideoSourceIntrinsics intrinsics;
	if (getCameraIntrinsics(intrinsics))
	{
		switch (intrinsics.intrinsics_type)
		{
			case MONO_CAMERA_INTRINSICS:
			{
				const MikanMonoIntrinsics& monoIntrinsics = intrinsics.getMonoIntrinsics();

				computeOpenGLProjMatFromCameraIntrinsics(
					monoIntrinsics,
					m_projectionMatrix);
			} break;
			case STEREO_CAMERA_INTRINSICS:
			{
				const MikanStereoIntrinsics& stereoIntrinsics = intrinsics.getStereoIntrinsics();

				computeOpenGLProjMatFromCameraIntrinsics(
					stereoIntrinsics,
					eStereoIntrinsicsSide::left,
					m_projectionMatrix);
			} break;
		}
	}
}

// -- IRmlPropertyInterface ----
void VideoSourceComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			VideoSourceDefinition::k_isFrameMirroredPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			VideoSourceDefinition::k_isBufferMirroredPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			VideoSourceDefinition::k_videoFrameQueueSizePropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			VideoSourceDefinition::k_hasValidIntrinsicsPropertyId)
		->setReadOnly());
}

bool VideoSourceComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc, 
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == VideoSourceDefinition::k_videoSourceIdPropertyId)
	{
		outValue = getVideoSourceId();
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_isFrameMirroredPropertyId)
	{
		outValue = getVideoSourceDefinition()->getIsFrameMirrored();
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_isBufferMirroredPropertyId)
	{
		outValue = getVideoSourceDefinition()->getIsBufferMirrored();
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_videoFrameQueueSizePropertyId)
	{
		outValue = getVideoSourceDefinition()->getVideoFrameQueueSize();
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_hasValidIntrinsicsPropertyId)
	{
		outValue = getVideoSourceDefinition()->hasCameraIntrinsics();
		return true;
	}

	return MikanComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool VideoSourceComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc, 
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == VideoSourceDefinition::k_isFrameMirroredPropertyId)
	{
		getVideoSourceDefinition()->setIsFrameMirrored(inValue.Get<bool>());
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_isBufferMirroredPropertyId)
	{
		getVideoSourceDefinition()->setIsBufferMirrored(inValue.Get<bool>());
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_videoFrameQueueSizePropertyId)
	{
		getVideoSourceDefinition()->setVideoFrameQueueSize(inValue.Get<int>());
		return true;
	}

	return MikanComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
const std::string VideoSourceComponent::k_deleteVideoSourceFunctionId = "delete_video_source";

void VideoSourceComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlFunctionDescriptor>(
			k_deleteVideoSourceFunctionId, "Delete Video Source"));
}

bool VideoSourceComponent::invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionName = functionDesc->getFunctionName();

	if (functionName == k_deleteVideoSourceFunctionId)
	{
		deleteVideoSource();
		return true;
	}

	return MikanComponent::invokeFunctionFromRml(functionDesc);
}

void VideoSourceComponent::deleteVideoSource()
{
	getOwnerObject()->deleteSelfConfig();
}