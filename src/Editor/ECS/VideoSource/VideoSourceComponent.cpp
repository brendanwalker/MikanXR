#include "CameraMath.h"
#include "IEditorWindow.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "MikanVideoSourceTypes.h"
#include "MonoLensCalibration/AppStage_MonoLensCalibration.h"
#include "VideoSourceSettings/AppStage_VideoSourceSettings.h"
#include "OpenCVVideoFrameBuffer.h"
#include "VideoSourceComponent.h"

#include <easy/profiler.h>

// -- VideoSourceDefinition -----
const std::string VideoSourceDefinition::k_videoSourceIntrinsicsPropertyId= "intrinsics_ptr";
const std::string VideoSourceDefinition::k_intrinsicsTypePropertyId = "intrinsics_type";
const std::string VideoSourceDefinition::k_isFrameMirroredPropertyId = "is_frame_mirrored";
const std::string VideoSourceDefinition::k_isBufferMirroredPropertyId = "is_buffer_mirrored";
const std::string VideoSourceDefinition::k_videoFrameQueueSizePropertyId = "video_frame_queue_size";

VideoSourceDefinition::VideoSourceDefinition()
	: MikanComponentDefinition()
	, m_intrinsics()
{}

VideoSourceDefinition::VideoSourceDefinition(
	MikanVideoSourceID videoSourceId)
	: MikanComponentDefinition(videoSourceId, "")
	, m_intrinsics()
{}

configuru::Config VideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt[VideoSourceDefinition::k_isFrameMirroredPropertyId] = m_bIsFrameMirrored;
	pt[VideoSourceDefinition::k_isBufferMirroredPropertyId] = m_bIsBufferMirrored;
	pt[VideoSourceDefinition::k_videoFrameQueueSizePropertyId] = m_videoFrameQueueSize;

	switch (m_intrinsics.intrinsics_type)
	{
		case MikanIntrinsicsType::MONO_CAMERA_INTRINSICS:
			pt["intrinsics_type"] = std::string("mono");
			CommonConfig::writeMonoTrackerIntrinsics(pt, m_intrinsics.getMonoIntrinsics());
			break;
		case MikanIntrinsicsType::STEREO_CAMERA_INTRINSICS:
			pt["intrinsics_type"] = std::string("stereo");
			CommonConfig::writeStereoTrackerIntrinsics(pt, m_intrinsics.getStereoIntrinsics());
			break;
	}

	return pt;
}

void VideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

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
		m_intrinsics.intrinsics_type = MikanIntrinsicsType::STEREO_CAMERA_INTRINSICS;

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
	notifyPropertyChanged(
		ConfigPropertyChangeSet()
		.addPropertyName(k_videoSourceIntrinsicsPropertyId)
		.addPropertyName(k_intrinsicsTypePropertyId));
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

// -- IEntityAccessor ----
rfk::Struct const* VideoSourceComponent::getClientAPIValuesStructType() const
{
	return &MikanVideoSourceValues::staticGetArchetype();
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
	if (m_opencv_buffer_state[(int)section] != nullptr)
	{
		int64_t lastFrameWriteIndex = m_opencv_buffer_state[(int)section]->getLastVideoFrameWriteIndex();

		return lastFrameWriteIndex != m_lastVideoFrameReadIndex;
	}

	return false;
}

int64_t VideoSourceComponent::readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer)
{
	EASY_FUNCTION();

	if (m_opencv_buffer_state[(int)section] != nullptr)
	{
		m_lastVideoFrameReadIndex =
			m_opencv_buffer_state[(int)section]->readVideoFrame(
				outBuffer,
				m_lastVideoFrameReadIndex);
	}

	return m_lastVideoFrameReadIndex;
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
	if (getVideoSourceDefinition()->getCameraIntrinsicsType() != MikanIntrinsicsType::INVALID_CAMERA_INTRINSICS)
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

bool VideoSourceComponent::setVideoSetting(const eVideoSettingType property_type, float desired_value)
{
	return false;
}

bool VideoSourceComponent::getVideoSetting(const eVideoSettingType property_type, float& outFractionValue) const
{
	return false;
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

	
	if (MikanVideoSourceIntrinsics intrinsics;
		getCameraIntrinsics(intrinsics))
	{
		// Allocate the OpenCV scratch buffers used for finding tracking blobs
		if (intrinsics.intrinsics_type == MikanIntrinsicsType::STEREO_CAMERA_INTRINSICS)
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
		else if (intrinsics.intrinsics_type == MikanIntrinsicsType::MONO_CAMERA_INTRINSICS)
		{
			const MikanMonoIntrinsics& monoIntrinsics = intrinsics.getMonoIntrinsics();

			m_opencv_buffer_state[(int)VideoFrameSection::Primary] =
				new OpenCVVideoFrameBuffer(
					videoPixelWidth, videoPixelHeight,
					monoIntrinsics.pixel_width, monoIntrinsics.pixel_width,
					VideoFrameSection::Primary);
		}
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
			case MikanIntrinsicsType::MONO_CAMERA_INTRINSICS:
			{
				const MikanMonoIntrinsics& monoIntrinsics = intrinsics.getMonoIntrinsics();

				computeOpenGLProjMatFromCameraIntrinsics(
					monoIntrinsics,
					m_projectionMatrix);
			} break;
			case MikanIntrinsicsType::STEREO_CAMERA_INTRINSICS:
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

// -- IPropertyInterface ----
void VideoSourceComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			VideoSourceDefinition::k_isFrameMirroredPropertyId, MikanVariantType::BOOL));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			VideoSourceDefinition::k_isBufferMirroredPropertyId, MikanVariantType::BOOL));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			VideoSourceDefinition::k_videoFrameQueueSizePropertyId, MikanVariantType::INT)
			->setDefaultValue(DEFAULT_VIDEO_FRAME_QUEUE_SIZE));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			VideoSourceDefinition::k_intrinsicsTypePropertyId, MikanVariantType::INT)
		->setReadOnly());
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(
			VideoSourceDefinition::k_videoSourceIntrinsicsPropertyId, MikanVariantType::POLYMORPHIC_OBJECT)
		->setReadOnly());	
}

bool VideoSourceComponent::getPropertyValue(
	const std::string& propertyName,
	MikanVariant& outValue) const
{
	if (propertyName == VideoSourceDefinition::k_isFrameMirroredPropertyId)
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
	else if (propertyName == VideoSourceDefinition::k_intrinsicsTypePropertyId)
	{
		outValue = (int)getVideoSourceDefinition()->getCameraIntrinsicsType();
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_videoSourceIntrinsicsPropertyId)
	{
		outValue = getVideoSourceDefinition()->getCameraIntrinsics().intrinsics_ptr;
		return true;
	}

	return MikanComponent::getPropertyValue(propertyName, outValue);
}

bool VideoSourceComponent::setPropertyValue(
	const std::string& propertyName,
	const MikanVariant& inValue)
{
	if (propertyName == VideoSourceDefinition::k_isFrameMirroredPropertyId)
	{
		getVideoSourceDefinition()->setIsFrameMirrored(inValue.getBoolValue());
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_isBufferMirroredPropertyId)
	{
		getVideoSourceDefinition()->setIsBufferMirrored(inValue.getBoolValue());
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_videoFrameQueueSizePropertyId)
	{
		getVideoSourceDefinition()->setVideoFrameQueueSize(inValue.getIntValue());
		return true;
	}

	return MikanComponent::setPropertyValue(propertyName, inValue);
}

// -- IFunctionInterface ----
const std::string VideoSourceComponent::k_showVideoSourceSettingsFunctionId = "show_video_source_settings";
const std::string VideoSourceComponent::k_calibrateIntrinsicsFunctionId = "calibrate_intrinsics";
const std::string VideoSourceComponent::k_testIntrinsicsFunctionId = "test_intrinsics";

void VideoSourceComponent::getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getFunctionDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_showVideoSourceSettingsFunctionId, "Show Video Source Settings"));
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_calibrateIntrinsicsFunctionId, "Calibrate Intrinsics"));
	outDescriptors.push_back(
		std::make_shared<FunctionDescriptor>(
			k_testIntrinsicsFunctionId, "Test Intrinsics"));
}

bool VideoSourceComponent::invokeFunction(FunctionDescriptorConstPtr functionDesc)
{
	const std::string& functionName = functionDesc->getFunctionName();

	if (functionName == k_showVideoSourceSettingsFunctionId)
	{
		showVideoSourceSettings();
		return true;
	}
	else if (functionName == k_calibrateIntrinsicsFunctionId)
	{
		calibrateIntrinsics();
		return true;
	}
	else if (functionName == k_testIntrinsicsFunctionId)
	{
		testIntrinsics();
		return true;
	}

	return MikanComponent::invokeFunction(functionDesc);
}

void VideoSourceComponent::showVideoSourceSettings()
{
	getOwnerEditorWindow()->pushAppStageOfType<AppStage_VideoSourceSettings>()
		->setVideoSourceComponent(getSelfPtr<VideoSourceComponent>());
}

void VideoSourceComponent::calibrateIntrinsics()
{
	getOwnerEditorWindow()->pushAppStageOfType<AppStage_MonoLensCalibration>();
}

void VideoSourceComponent::testIntrinsics()
{
	getOwnerEditorWindow()->pushAppStageOfType<AppStage_MonoLensCalibration>()
		->setBypassCalibrationFlag(true);
}