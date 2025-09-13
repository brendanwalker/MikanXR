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
	void setClientSource(const std::string& spoutSource);

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

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
	{ VideoSourceComponent::getPropertyNamesStatic(outPropertyNames); }

	// -- IFunctionInterface ----
	static void getFunctionNamesStatic(std::vector<std::string>& outPropertyNames)
	{ VideoSourceComponent::getFunctionNamesStatic(outPropertyNames); }
};