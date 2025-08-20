#include "CameraMath.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "VideoSourceComponent.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

// -- VideoSourceDefinition -----
const std::string VideoSourceDefinition::k_videoSourceIdPropertyId = "video_source_id";
const std::string VideoSourceDefinition::k_videoSourceIntrinsicsPropertyId= "video_source_intrinsics";
const std::string VideoSourceDefinition::k_isFrameMirroredPropertyId = "is_frame_mirrored";
const std::string VideoSourceDefinition::k_isBufferMirroredPropertyId = "is_buffer_mirrored";
const std::string VideoSourceDefinition::k_videoFrameQueueSizePropertyId = "video_frame_queue_size";

VideoSourceDefinition::VideoSourceDefinition()
	: MikanComponentDefinition()
	, m_videoSourceId(INVALID_MIKAN_ID)
{}

VideoSourceDefinition::VideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const std::string& videoSourceName)
	: MikanComponentDefinition(videoSourceName)
	, m_videoSourceId(videoSourceId)
{}

VideoSourceDefinition::VideoSourceDefinition(
	MikanVideoSourceID videoSourceId,
	const std::string& videoSourceName,
	const MikanVideoSourceIntrinsics& intrinsics)
	: MikanComponentDefinition(videoSourceName)
	, m_videoSourceId(videoSourceId)
	, m_intrinsics(intrinsics)
{}

configuru::Config VideoSourceDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["video_source_id"] = m_videoSourceId;
	pt["is_frame_mirrored"] = m_bIsFrameMirrored;
	pt["is_buffer_mirrored"] = m_bIsBufferMirrored;
	pt["video_frame_queue_size"] = m_videoFrameQueueSize;

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
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_isFrameMirroredPropertyId));
}

void VideoSourceDefinition::setIsBufferMirrored(bool isBufferMirrored)
{
	m_bIsBufferMirrored = isBufferMirrored;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_isBufferMirroredPropertyId));
}

void VideoSourceDefinition::setVideoFrameQueueSize(int videoFrameQueueSize)
{
	if (m_videoFrameQueueSize != videoFrameQueueSize)
	{
		m_videoFrameQueueSize = videoFrameQueueSize;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_videoFrameQueueSizePropertyId));
	}
}

void VideoSourceDefinition::setCameraIntrinsics(
	const MikanVideoSourceIntrinsics& cameraIntrinsics)
{
	m_intrinsics = cameraIntrinsics;
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_videoSourceIntrinsicsPropertyId));
}

// -- VideoSourceComponent -----
const std::string VideoSourceComponent::k_deleteVideoSourceFunctionId = "delete_video_source";

VideoSourceComponent::VideoSourceComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
	, m_projectionMatrix(glm::mat4(1.f))
{
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

bool VideoSourceComponent::getVideoModeName(std::string& outVideoModeName) const
{
	return false;
}

bool VideoSourceComponent::getPixelDimensions(int& outPixelWidth, int& outPixelHeight) const
{
	MikanVideoSourceIntrinsics intrinsics;
	if (getCameraIntrinsics(intrinsics))
	{
		switch (intrinsics.intrinsics_type)
		{
			case MONO_CAMERA_INTRINSICS:
			{
				const MikanMonoIntrinsics& monoIntrinsics = intrinsics.getMonoIntrinsics();
				outPixelWidth = (int)monoIntrinsics.pixel_width;
				outPixelHeight = (int)monoIntrinsics.pixel_height;

				return true;
			}
			case STEREO_CAMERA_INTRINSICS:
			{
				const MikanStereoIntrinsics& stereoIntrinsics = intrinsics.getStereoIntrinsics();
				outPixelWidth = (int)stereoIntrinsics.pixel_width;
				outPixelHeight = (int)stereoIntrinsics.pixel_height;

				return true;
			}
		}
	}

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

// -- IPropertyInterface ----
void VideoSourceComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(VideoSourceDefinition::k_videoSourceIdPropertyId);
	outPropertyNames.push_back(VideoSourceDefinition::k_isFrameMirroredPropertyId);
	outPropertyNames.push_back(VideoSourceDefinition::k_isBufferMirroredPropertyId);
}

bool VideoSourceComponent::getPropertyDescriptor(
	const std::string& propertyName, 
	PropertyDescriptor& outDescriptor) const
{
	if (MikanComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == VideoSourceDefinition::k_videoSourceIdPropertyId)
	{
		outDescriptor = {VideoSourceDefinition::k_videoSourceIdPropertyId, ePropertyDataType::datatype_int, ePropertySemantic::video_source_id};
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_isFrameMirroredPropertyId)
	{
		outDescriptor = { VideoSourceDefinition::k_isFrameMirroredPropertyId, ePropertyDataType::datatype_bool, ePropertySemantic::checkbox };
		return true;
	}
	else if (propertyName == VideoSourceDefinition::k_isBufferMirroredPropertyId)
	{
		outDescriptor = { VideoSourceDefinition::k_isBufferMirroredPropertyId, ePropertyDataType::datatype_bool, ePropertySemantic::checkbox };
		return true;
	}

	return false;
}

bool VideoSourceComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyValue(propertyName, outValue))
		return true;

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

	return false;
}

bool VideoSourceComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (MikanComponent::setPropertyValue(propertyName, inValue))
		return true;

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

	return false;
}

// -- IFunctionInterface ----
void VideoSourceComponent::getFunctionNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getFunctionNames(outPropertyNames);

	outPropertyNames.push_back(k_deleteVideoSourceFunctionId);
}

bool VideoSourceComponent::getFunctionDescriptor(
	const std::string& functionName, 
	FunctionDescriptor& outDescriptor) const
{
	if (MikanComponent::getFunctionDescriptor(functionName, outDescriptor))
		return true;

	if (functionName == k_deleteVideoSourceFunctionId)
	{
		outDescriptor = {k_deleteVideoSourceFunctionId, "Delete Video Source"};
		return true;
	}

	return false;
}

bool VideoSourceComponent::invokeFunction(const std::string& functionName)
{
	if (MikanComponent::invokeFunction(functionName))
		return true;

	if (functionName == k_deleteVideoSourceFunctionId)
	{
		deleteVideoSource();
		return true;
	}

	return false;
}

void VideoSourceComponent::deleteVideoSource()
{
	getOwnerObject()->deleteSelfConfig();
}