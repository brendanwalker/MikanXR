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
		bool bBroadcastPropertyChange = true);
	void notifyCameraSettingsChanged();

private:
	std::string m_devicePath;
	std::string m_videoMode;
	std::map<std::string, std::array<float, (int)eVideoSettingType::COUNT> > m_videoSettingsMap;
};

class USBVideoSourceComponent : public VideoSourceComponent, public IUsbVideoDeviceListener
{
public:
	USBVideoSourceComponent(MikanObjectWeakPtr owner);

	virtual void init() override;
	virtual void dispose() override;

	inline USBVideoSourceDefinitionPtr getUSBVideoSourceDefinition() const
	{
		return std::static_pointer_cast<USBVideoSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	inline static const std::string k_componentClassName = "USBVideoSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	std::string getDeviceFriendlyName() const;

	// VideoSourceComponent Interface
	virtual std::string getDevicePath() const override;
	virtual std::string getDeviceAPI() const override;
	virtual bool openVideoSource() override;
	virtual void closeVideoSource() override;
	virtual eVideoStreamingStatus startVideoStream() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;
	virtual bool getVideoModeName(std::string& outVideoModeName) const override;
	virtual bool getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const override;
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
	bool getVideoModeNames(std::vector<std::string>& outVideoModeNames) const;

	// -- IUsbVideoDeviceListener ----
	virtual void notifyVideoDeviceDisconnected(const IUsbVideoDevice* device) override;
	virtual void notifyVideoModePropertiesChanged(const IUsbVideoDevice* device) override;
	virtual void notifyVideoFrameReceived(const UsbVideoFrameBuffer& bufferInfo) override;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

protected:
	void onDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet) override;
	bool updateVideoMode();
	void updateCameraSettings();

protected:
	IUsbVideoDevice* m_usbVideoDevice;
};