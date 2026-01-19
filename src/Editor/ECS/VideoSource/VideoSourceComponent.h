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
#include "ProjectConfigConstants.h"
#include "VideoDisplayConstants.h"

#include <map>
#include <memory>
#include <string>

class VideoSourceDefinition : public MikanComponentDefinition
{
public:
	VideoSourceDefinition();
	VideoSourceDefinition(MikanVideoSourceID videoSourceId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	inline MikanVideoSourceID getVideoSourceId() const { return getComponentId(); }

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
	inline const MikanVideoSourceIntrinsics& getCameraIntrinsics() const { return m_intrinsics; }
	void setCameraIntrinsics(const MikanVideoSourceIntrinsics& cameraIntrinsics);

	static const std::string k_intrinsicsTypePropertyId;
	inline MikanIntrinsicsType getCameraIntrinsicsType() const { return m_intrinsics.intrinsics_type; }

private:
	bool m_bIsFrameMirrored= false;
	bool m_bIsBufferMirrored= false;
	int m_videoFrameQueueSize = DEFAULT_VIDEO_FRAME_QUEUE_SIZE;
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

	inline static const std::string k_componentClassName = "VideoSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	MikanVideoSourceID getVideoSourceId() const;
	
	// Video Source Interface
	virtual std::string getDevicePath() const = 0;
	virtual std::string getDeviceAPI() const = 0;
	virtual bool openVideoSource() = 0;
	virtual void closeVideoSource() = 0;
	virtual eVideoStreamingStatus startVideoStream() = 0;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const = 0;
	virtual void stopVideoStream() = 0;

	virtual bool getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const;
	virtual bool hasNewVideoFrameAvailable(VideoFrameSection section) const;
	virtual int64_t readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer);

	virtual bool getVideoModeName(std::string& outVideoModeName) const;
	virtual bool getFrameRate(float& outFrameRate) const;
	virtual bool getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const;
	virtual bool setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics);
	virtual glm::mat4 getProjectionMatrix() const;

	virtual bool isVideoSettingSupported(const eVideoSettingType property_type) const;
	virtual bool setVideoSetting(const eVideoSettingType property_type, float desiredFraction);
	virtual bool getVideoSetting(const eVideoSettingType property_type, float& outFractionValue) const;

	// Video Source Events
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnOpened;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnClosed;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnStarted;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnStopped;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnFrameSizeChanged;
	MulticastDelegate<void(VideoSourceComponentPtr videoSource)> OnIntrinsicsChanged;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_deleteVideoSourceFunctionId;
	static const std::string k_showVideoSourceSettingsFunctionId;
	static const std::string k_calibrateIntrinsicsFunctionId;
	static const std::string k_testIntrinsicsFunctionId;
	static void getFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunction(FunctionDescriptorConstPtr functionDesc) override;

	void deleteVideoSource();
	void showVideoSourceSettings();
	void calibrateIntrinsics();
	void testIntrinsics();

protected:
	bool hasAllocatedOpencvBufferState() const;
	bool reallocateOpencvBufferState();
	void releaseOpencvBufferState();
	void recomputeCameraProjectionMatrix();

protected:
	int64_t m_lastVideoFrameReadIndex;
	class OpenCVVideoFrameBuffer* m_opencv_buffer_state[MAX_PROJECTION_COUNT];
	glm::mat4 m_projectionMatrix;
};