#pragma once

#include "IARKitVideoDevice.h"
#include "VideoSourceComponent.h"

class ARKitVideoSourceDefinition : public VideoSourceDefinition
{
public:
	ARKitVideoSourceDefinition();
	ARKitVideoSourceDefinition(MikanVideoSourceID videoSourceId);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
	virtual bool readFromInitParams(MikanObjectSystem* ownerObjectSystem,
									const Serialization::PolymorphicObjectPtr& initParams) override;

	// Stored as int (not uint16_t) so getPropertyValue()'s static-type-inferred
	// MikanVariantType matches the reflected MikanARKitVideoSourceValues::base_port
	// field (an int) exactly - see ServerEntitySerializer's per-type rules. Narrowed
	// to uint16_t only at the ARKitVideoConnectionSettings boundary in
	// openVideoSource().
	static const std::string k_basePortPropertyId;
	inline int getBasePort() const { return m_basePort; }
	void setBasePort(int basePort);

	static const std::string k_depthStreamingEnabledPropertyId;
	inline bool getDepthStreamingEnabled() const { return m_bDepthStreamingEnabled; }
	void setDepthStreamingEnabled(bool depthStreamingEnabled);

	// Joint Bilateral Upsampling tuning params (ticket D5's JBUParams defaults - see
	// MikanARKitVideo/Private/Cuda/JBUKernel.h). Not yet consumed by the live video
	// pipeline as of ticket E1 - MikanARKitVideoDevice doesn't have a depth-upsample
	// stage wired in yet (that's a later ticket's job, alongside the depth-texture
	// slot mentioned in ticket E3); these are stored/exposed here now so client
	// tooling and the future depth pipeline have a single source of truth to read
	// from once it lands.
	static const std::string k_jbuRadiusPropertyId;
	inline int getJbuRadius() const { return m_jbuRadius; }
	void setJbuRadius(int jbuRadius);

	static const std::string k_jbuSigmaSpatialPropertyId;
	inline float getJbuSigmaSpatial() const { return m_jbuSigmaSpatial; }
	void setJbuSigmaSpatial(float jbuSigmaSpatial);

	static const std::string k_jbuSigmaColorPropertyId;
	inline float getJbuSigmaColor() const { return m_jbuSigmaColor; }
	void setJbuSigmaColor(float jbuSigmaColor);

	static const std::string k_jbuConfWeightLowPropertyId;
	inline float getJbuConfWeightLow() const { return m_jbuConfWeightLow; }
	void setJbuConfWeightLow(float jbuConfWeightLow);

	static const std::string k_jbuConfWeightMediumPropertyId;
	inline float getJbuConfWeightMedium() const { return m_jbuConfWeightMedium; }
	void setJbuConfWeightMedium(float jbuConfWeightMedium);

private:
	int m_basePort;
	bool m_bDepthStreamingEnabled;

	int m_jbuRadius;
	float m_jbuSigmaSpatial;
	float m_jbuSigmaColor;
	float m_jbuConfWeightLow;
	float m_jbuConfWeightMedium;
};

class ARKitVideoSourceComponent : public VideoSourceComponent, public IARKitVideoDeviceListener
{
public:
	ARKitVideoSourceComponent(MikanObjectWeakPtr owner);

	virtual void init() override;
	virtual void dispose() override;

	inline static const std::string k_componentClassName= "ARKitVideoSourceComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline ARKitVideoSourceDefinitionPtr getARKitVideoSourceDefinition() const
	{
		return std::static_pointer_cast<ARKitVideoSourceDefinition>(m_definition);
	}
	virtual void setDefinition(MikanComponentDefinitionPtr definition) override;

	// -- Video Source Interface ----
	virtual std::string getDevicePath() const override;
	virtual std::string getDeviceAPI() const override;
	virtual bool openVideoSource() override;
	virtual void closeVideoSource() override;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual bool getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const override;

	// -- IARKitVideoDeviceListener ----
	virtual void notifyDeviceOpened(const class IARKitVideoDevice* device) override;
	virtual void notifyDeviceClosed(const class IARKitVideoDevice* device) override;
	virtual void notifyFrameBundleReceived(const ARKitVideoFrameBundle& bundle) override;

	// -- IEntityAccessor ----
	virtual rfk::Struct const* getClientAPIValuesStructType() const override;

	// -- IPropertyInterface ----
	static void getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const override;
	virtual bool setPropertyValue(const std::string& propertyName, const MikanVariant& inValue) override;

	bool isPendingOpen() const { return m_bPendingOpen; }

protected:
	virtual eVideoStreamingStatus startVideoStreamInternal() override;
	virtual void stopVideoStreamInternal() override;

	void onDefinitionMarkedDirty(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet);

private:
	IARKitVideoDevicePtr m_arkitVideoDevice= nullptr;
	bool m_bPendingOpen= false;

	// Cached from the most recently received frame bundle - IARKitVideoDevice has no
	// getStreamProperties()-style query the way INetworkVideoDevice does (see
	// NetworkVideoSourceComponent), since ARKit's video dimensions are only known
	// once a bundle actually arrives.
	int m_lastVideoWidth= 0;
	int m_lastVideoHeight= 0;
};
