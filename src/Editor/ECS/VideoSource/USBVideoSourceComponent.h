#pragma once

#include "VideoSourceComponent.h"
#include "IUsbVideoDevice.h"
#include "MulticastDelegate.h"

using USBVideoSettingsArray = std::array<float, (int)eVideoSettingType::COUNT>;
using USBVideoConstraintArray = std::array<VideoSettingConstraint, (int)eVideoSettingType::COUNT>;

class USBVideoSourceDefinition : public VideoSourceDefinition
{
public:
	USBVideoSourceDefinition();
	USBVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const MikanUSBVideoSourceInfo& videoSourceInfo);

	virtual bool wantsSaveForPropertyChange(const ConfigPropertyChangeSet& changedPropertySet) const;
	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_desiredDevicePathPropertyId;
	inline const std::string& getDevicePath() const { return m_devicePath; }
	void setDevicePath(const std::string& devicePath);

	static const std::string k_videoModePropertyId;
	inline const std::string& getVideoMode() const { return m_videoMode; }
	void setVideoMode(const std::string& videoMode);

	static const std::string k_videoSettingsPropertyId;
	bool getVideoSettingsForMode(
		const std::string& modeName,
		USBVideoSettingsArray& outSettings) const;
	void setCameraSettingsForMode(
		const std::string& modeName,
		const USBVideoSettingsArray& settings);
	bool hasVideoSettingsForMode(const std::string& videoModeName) const;
	void notifyCameraSettingsChanged();

private:
	std::string m_devicePath;
	std::string m_videoMode;
	std::map<std::string, USBVideoSettingsArray> m_videoSettingsMap;
};

class USBVideoSourceComponent : public VideoSourceComponent, public IUsbVideoDeviceListener
{
public:
	USBVideoSourceComponent(MikanObjectWeakPtr owner);

	virtual void init() override;
	virtual void update(float deltaSeconds) override;
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
	virtual bool setVideoSetting(const eVideoSettingType property_type, float desiredFraction) override;
	virtual bool getVideoSetting(const eVideoSettingType property_type, float& outFractionValue) const override;
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
	static const std::string k_currentDevicePathPropertyId;
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, MikanVariant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const MikanVariant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_resetToDefaultsFunctionId;
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outPropertyNames);
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) override;

protected:
	void onDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet) override;
	bool updateVideoMode();
	int findBestVideoModeIndex(int w, int h, int frameRate) const;
	bool handleVideoModeUpdated();
	void handleVideoModeSettingUpdated();
	void saveVideoSettingDefaultsFromCurrentMode();
	void restoreVideoSettingsToCurrentMode();
	bool getVideoSettingAsFloatFraction(eVideoSettingType settingType, float& outFloatFraction) const;
	bool setVideoSettingAsFloatFraction(eVideoSettingType settingType, float outFloatFraction, bool bForce= false);

protected:
	IUsbVideoDevice* m_usbVideoDevice;
	USBVideoSettingsArray m_currentVideoSettings;
	USBVideoConstraintArray m_currentVideoConstraints;
	bool m_bDeviceChanged= false;
	bool m_bModeChanged = false;
	bool m_bSettingsChanged = false;
};