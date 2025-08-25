#pragma once

#include "VideoSourceComponent.h"

class NetworkVideoSourceDefinition : public VideoSourceDefinition
{
public:
	NetworkVideoSourceDefinition();
	NetworkVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const MikanNetworkVideoSourceInfo& videoSourceInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_urlPropertyId;
	inline const std::string& getURL() const { return m_url; }
	void setURL(const std::string& url);

private:
	std::string m_url;
};

class NetworkVideoSourceComponent : public VideoSourceComponent
{
public:
	NetworkVideoSourceComponent(MikanObjectWeakPtr owner);

	inline NetworkVideoSourceDefinitionPtr getNetworkVideoSourceDefinition() const
	{
		return std::static_pointer_cast<NetworkVideoSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	// Video Source Interface
	virtual std::string getDevicePath() const override;
	virtual std::string getDeviceAPI() const override;
	virtual bool openVideoSource() override;
	virtual void closeVideoSource() override;
	virtual eVideoStreamingStatus startVideoStream() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;

	virtual bool hasNewVideoFrameAvailable(VideoFrameSection section) const override;
	virtual int64_t readVideoFrameSectionBuffer(VideoFrameSection section, cv::Mat* outBuffer) override;

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
};