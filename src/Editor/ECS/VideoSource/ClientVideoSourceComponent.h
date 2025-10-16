#pragma once

#include "VideoSourceComponent.h"

class ClientVideoSourceDefinition : public VideoSourceDefinition
{
public:
	ClientVideoSourceDefinition();
	ClientVideoSourceDefinition(
		MikanVideoSourceID videoSourceId,
		const struct MikanClientVideoSourceInfo& videoSourceInfo);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_clientSourcePropertyId;
	inline const std::string& getClientSource() const { return m_clientSource; }
	void setClientSource(const std::string& clientSource);

private:
	std::string m_clientSource;
};

class ClientVideoSourceComponent : public VideoSourceComponent
{
public:
	ClientVideoSourceComponent(MikanObjectWeakPtr owner);

	inline static const std::string k_componentClassName = "ClientVideoSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline ClientVideoSourceDefinitionPtr getClientVideoSourceDefinition() const
	{
		return std::static_pointer_cast<ClientVideoSourceDefinition>(m_definition);
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

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors) 
	{ VideoSourceComponent::getRmlPropertyDescriptors(outDescriptors); }

	// -- IRmlFunctionInterface ----
	static void getFunctionNamesStatic(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
	{ VideoSourceComponent::getRmlFunctionDescriptors(outDescriptors); }
};