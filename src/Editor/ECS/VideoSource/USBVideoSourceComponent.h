#pragma once

#include "VideoSourceComponent.h"
#include "IUsbVideoDevice.h"
#include "MulticastDelegate.h"

class USBVideoSourceDefinition : public VideoSourceDefinition
{
public:
	USBVideoSourceDefinition();
	USBVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const MikanUSBVideoSourceInfo& videoSourceInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_devicePathPropertyId;
	inline const std::string& getDevicePath() const { return m_devicePath; }
	void setDevicePath(const std::string& devicePath);

	static const std::string k_videoModePropertyId;
	inline const std::string& getVideoMode() const { return m_videoMode; }
	void setVideoMode(const std::string& videoMode);

	static const std::string k_cameraSettingsPropertyId;
	bool getVideoSettingValue(
		const std::string& modeName, 
		eVideoSettingType settingType,
		float& outValue) const;
	void setCameraSettingValue(
		const std::string& modeName,
		eVideoSettingType settingType,
		float value,
		bool bBroadcastMarkDirty = true);
	void markCameraSettingsDirty();

private:
	std::string m_devicePath;
	std::string m_videoMode;
	std::map<std::string, std::array<float, (int)VideoPropertyType::COUNT> > m_videoSettingsMap;
};

class USBVideoSourceComponent : public VideoSourceComponent, public IUsbVideoDeviceListener
{
public:
	USBVideoSourceComponent(MikanObjectWeakPtr owner);

	virtual void dispose() override;

	inline USBVideoSourceDefinitionPtr getUSBVideoSourceDefinition() const
	{
		return std::static_pointer_cast<USBVideoSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	// VideoSourceComponent Interface
	virtual std::string getDevicePath() const override;
	virtual std::string getDeviceAPI() const override;
	virtual bool openVideoSource() override;
	virtual void closeVideoSource() override;
	virtual eVideoStreamingStatus startVideoStream() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;
	virtual bool hasNewVideoFrameAvailable(VideoFrameSection section) const override;
	virtual int64_t readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer) override;	
	virtual bool getVideoModeName(std::string& outVideoModeName) const override;
	virtual bool isVideoSettingSupported(const eVideoSettingType property_type) const override;
	virtual bool getVideoSettingConstraint(const eVideoSettingType property_type, VideoSettingConstraint& outConstraint) const override;
	virtual void setVideoSetting(const eVideoSettingType property_type, int desired_value) override;
	virtual int getVideoSetting(const eVideoSettingType property_type) const override;
	virtual bool getFrameRate(float& outFrameRate) const override;

	// -- USB Video Mode
	size_t getAvailableVideoModesCount() const;
	bool getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const;
	int getVideoModeIndex() const;
	bool setVideoModeByName(const std::string& videoModeName);
	bool setVideoModeByIndex(size_t index);

	// -- IUsbVideoDeviceListener ----
	virtual void notifyVideoDeviceDisconnected(const IUsbVideoDevice* device) override;
	virtual void notifyVideoModePropertiesChanged(const IUsbVideoDevice* device) override;
	virtual void notifyVideoFrameReceived(const UsbVideoFrameBuffer& bufferInfo) override;

	// -- IPropertyInterface ----
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const override;
	virtual bool getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue) override;

	// -- IFunctionInterface ----
	static const std::string k_calibrateIntrinsicsFunctionId;
	static const std::string k_testIntrinsicsFunctionId;
	virtual void getFunctionNames(std::vector<std::string>& outPropertyNames) const override;
	virtual bool getFunctionDescriptor(const std::string& functionName, FunctionDescriptor& outDescriptor) const override;
	virtual bool invokeFunction(const std::string& functionName) override;

	void calibrateIntrinsics();
	void testIntrinsics();

protected:
	bool updateVideoMode();
	void updateCameraSettings();
	bool reallocateOpencvBufferState();
	void releaseOpencvBufferState();
	void recomputeCameraProjectionMatrix();	

protected:
	int64_t m_lastVideoFrameReadIndex;
	class OpenCVVideoFrameBuffer* m_opencv_buffer_state[MAX_PROJECTION_COUNT];
	IUsbVideoDevice* m_usbVideoDevice;
	glm::mat4 m_projectionMatrix;
};