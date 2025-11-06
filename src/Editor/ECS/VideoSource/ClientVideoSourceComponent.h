#pragma once

#include "CompositorConstants.h"
#include "VideoSourceComponent.h"
#include "MkRendererFwd.h"

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

	// Client Video Source Interface
	IMkTexturePtr getClientColorSourceTexture(eClientColorTextureType clientTextureType) const;
	IMkTexturePtr getClientDepthSourceTexture(eClientDepthTextureType clientTextureType) const;

	// Video Source Interface
	virtual std::string getDevicePath() const override;
	virtual std::string getDeviceAPI() const override;
	virtual bool openVideoSource() override;
	virtual void closeVideoSource() override;
	virtual eVideoStreamingStatus startVideoStream() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;

	virtual bool getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const override;

	//TODO
	// -- IClientVideoSourceListener ----
	//virtual void notifyClientVideoSourceClosed(const class INetworkVideoDevice* device) override;
	//virtual void notifyClientVideoFrameReceived(const NetworkVideoFrameBuffer& bufferInfo) override;

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static void getFunctionNamesStatic(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
	{ VideoSourceComponent::getRmlFunctionDescriptors(outDescriptors); }

protected:
	class ClientSourceManager* getClientSourceManager() const;
	const std::string& getClientSourceName() const;
};