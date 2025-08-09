#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "MikanVideoSourceTypes.h"
#include "OpenCVFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "VideoSourceInterface.h"

#include <map>
#include <memory>
#include <string>

#define LEFT_PROJECTION_INDEX  0
#define RIGHT_PROJECTION_INDEX 1

#define MONO_PROJECTION_COUNT 1
#define STEREO_PROJECTION_COUNT 2

#define MAX_PROJECTION_COUNT 2
#define PRIMARY_PROJECTION_INDEX LEFT_PROJECTION_INDEX

class VideoSourceDefinition : public MikanComponentDefinition
{
public:
	VideoSourceDefinition();
	VideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const std::string& videoSourceName);
	VideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const std::string& videoSourceName,
		const MikanVideoSourceIntrinsics& intrinsics);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_videoSourceIdPropertyId;
	inline MikanVideoSourceID getVideoSourceId() const { return m_videoSourceId; }

	static const std::string k_isFrameMirroredPropertyId;
	inline bool getIsFrameMirrored() const { return m_bIsFrameMirrored; }
	void setIsFrameMirrored(bool isFrameMirrored);

	static const std::string k_isBufferMirroredPropertyId;
	inline bool getIsBufferMirrored() const { return m_bIsBufferMirrored; }
	void setIsBufferMirrored(bool isBufferMirrored);

	static const std::string k_videoSourceIntrinsicsPropertyId;
	inline bool hasCameraIntrinsics() const { return m_intrinsics.intrinsics_type != INVALID_CAMERA_INTRINSICS; }
	inline const MikanVideoSourceIntrinsics& getCameraIntrinsics() const { return m_intrinsics; }
	void setCameraIntrinsics(const MikanVideoSourceIntrinsics& cameraIntrinsics);

private:
	MikanVideoSourceID m_videoSourceId;
	bool m_bIsFrameMirrored;
	bool m_bIsBufferMirrored;
	MikanVideoSourceIntrinsics m_intrinsics;
};

class VideoSourceComponent : public MikanComponent
{
public:
	VideoSourceComponent(MikanObjectWeakPtr owner);

	inline VideoSourceDefinitionPtr getVideoSourceDefinition() const
	{
		return std::static_pointer_cast<VideoSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	MikanVideoSourceID getVideoSourceId() const;
	
	// Video Source Interface
	virtual bool openVideoSource() = 0;
	virtual void closeVideoSource() = 0;
	virtual eVideoStreamingStatus startVideoStream() = 0;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const = 0;
	virtual void stopVideoStream() = 0;

	virtual bool hasNewVideoFrameAvailable(VideoFrameSection section) const = 0;
	virtual int64_t readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer) = 0;

	virtual bool getPixelDimensions(int& outPixelWidth, int& outPixelHeight) const;
	virtual bool getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const;
	virtual bool setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics);

	// Video Source Events
	MulticastDelegate<void(const VideoSourceComponent* videoSource)> OnOpened;
	MulticastDelegate<void(const VideoSourceComponent* videoSource)> OnClosed;
	MulticastDelegate<void(const VideoSourceComponent* videoSource)> OnStarted;
	MulticastDelegate<void(const VideoSourceComponent* videoSource)> OnStopped;
	MulticastDelegate<void(const VideoSourceComponent* videoSource)> OnFrameSizeChanged;
	MulticastDelegate<void(const VideoSourceComponent* videoSource)> OnIntrinsicsChanged;

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_deleteVideoSourceFunctionId;
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	void deleteVideoSource();
};