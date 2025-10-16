#pragma once

#include "VideoSourceComponent.h"

class SpoutVideoSourceDefinition : public VideoSourceDefinition
{
public:
	SpoutVideoSourceDefinition();
	SpoutVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const MikanSpoutVideoSourceInfo& videoSourceInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_spoutSourcePropertyId;
	inline const std::string& getSpoutSource() const { return m_spoutSource; }
	void setSpoutSource(const std::string& spoutSource);

private:
	std::string m_spoutSource;
};

class SpoutVideoSourceComponent : public VideoSourceComponent
{
public:
	SpoutVideoSourceComponent(MikanObjectWeakPtr owner);

	inline SpoutVideoSourceDefinitionPtr getSpoutVideoSourceDefinition() const
	{
		return std::static_pointer_cast<SpoutVideoSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	inline static const std::string k_componentClassName = "SpoutVideoSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

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

	virtual bool getPixelDimensions(int& outPixelWidth, int& outPixelHeight) const override;
	virtual bool getCameraIntrinsics(MikanVideoSourceIntrinsics& out_camera_intrinsics) const override;
	virtual bool setCameraIntrinsics(const MikanVideoSourceIntrinsics& camera_intrinsics) override;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
	{ VideoSourceComponent::getRmlFunctionDescriptors(outDescriptors); }
};