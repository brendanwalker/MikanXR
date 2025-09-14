#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "IVideoDevice.h"
#include "MikanComponent.h"
#include "MikanTypeFwd.h"
#include "MikanVideoSourceTypes.h"
#include "OpenCVFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectFwd.h"
#include "VideoDisplayConstants.h"

#include <map>
#include <memory>
#include <string>

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

	static const std::string k_videoFrameQueueSizePropertyId;
	inline int getVideoFrameQueueSize() const { return m_videoFrameQueueSize; }
	void setVideoFrameQueueSize(int videoFrameQueueSize);

	static const std::string k_videoSourceIntrinsicsPropertyId;
	inline bool hasCameraIntrinsics() const { return m_intrinsics.intrinsics_type != INVALID_CAMERA_INTRINSICS; }
	inline const MikanVideoSourceIntrinsics& getCameraIntrinsics() const { return m_intrinsics; }
	void setCameraIntrinsics(const MikanVideoSourceIntrinsics& cameraIntrinsics);

private:
	MikanVideoSourceID m_videoSourceId;
	bool m_bIsFrameMirrored;
	bool m_bIsBufferMirrored;
	int m_videoFrameQueueSize;
	MikanVideoSourceIntrinsics m_intrinsics;
};

class VideoSourceComponent : public MikanComponent
{
public:
	VideoSourceComponent(MikanObjectWeakPtr owner);

	inline VideoSourceDefinitionPtr getVideoSourceDefinition() const
	{
		return std::static_pointer_cast<VideoSourceDefinition>(getDefinition());
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	MikanVideoSourceID getVideoSourceId() const;
	
	// Video Source Interface
	virtual std::string getDevicePath() const = 0;
	virtual std::string getDeviceAPI() const = 0;
	virtual bool openVideoSource() = 0;
	virtual void closeVideoSource() = 0;
	virtual eVideoStreamingStatus startVideoStream() = 0;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const = 0;
	virtual void stopVideoStream() = 0;

	virtual bool hasNewVideoFrameAvailable(VideoFrameSection section) const = 0;
	virtual int64_t readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer) = 0;

	virtual bool getVideoModeName(std::string& outVideoModeName) const;
	virtual bool getPixelDimensions(int& outPixelWidth, int& outPixelHeight) const;
	virtual bool getFrameRate(float& outFrameRate) const;
	virtual bool getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const;
	virtual bool setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics);
	virtual glm::mat4 getProjectionMatrix() const;

	virtual bool isVideoSettingSupported(const eVideoSettingType property_type) const;
	virtual bool getVideoSettingConstraint(const eVideoSettingType property_type, VideoSettingConstraint& outConstraint) const;
	virtual void setVideoSetting(const eVideoSettingType property_type, int desired_value);
	virtual int getVideoSetting(const eVideoSettingType property_type) const;

	// Video Source Events
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnOpened;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnClosed;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnStarted;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnStopped;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnFrameSizeChanged;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnIntrinsicsChanged;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_deleteVideoSourceFunctionId;
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) override;

	void deleteVideoSource();

protected:
	void recomputeCameraProjectionMatrix();

private:
	glm::mat4 m_projectionMatrix;
};