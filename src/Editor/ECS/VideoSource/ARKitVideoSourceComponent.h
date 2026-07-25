#pragma once

#include "IARKitVideoDevice.h"
#include "IFrameCoupledPoseProvider.h"
#include "MikanVideoSourceTypes.h"
#include "MkRendererFwd.h"
#include "VideoSourceComponent.h"

#include <atomic>
#include <mutex>

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
	// MikanARKitVideo/Private/Cuda/JBUKernel.h). Live-applied to the running device
	// via ARKitVideoSourceComponent::pushJBUParamsToDevice() - both on device-open
	// and on every subsequent edit (see that component's onDefinitionMarkedDirty),
	// so changing these at runtime takes effect immediately without a reconnect.
	// Setters clamp to safe ranges (see the .cpp) since sigmaSpatial/sigmaColor of
	// exactly 0 would divide-by-zero in the kernel.
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

class ARKitVideoSourceComponent : public VideoSourceComponent,
								  public IARKitVideoDeviceListener,
								  public IFrameCoupledPoseProvider
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

	// -- Zero-copy GPU texture access (ticket E3) ----
	virtual IMkTexturePtr getDirectColorTexture() const override;
	virtual IMkTexturePtr getDirectDepthTexture() const override;

	// Latest bundle's frameSeq (video, depth, or pose - whichever arrived most
	// recently), or -1 if none has arrived yet (ticket E4 - see
	// VideoSourceComponent::getDirectFrameIndex()'s own comment for why this
	// exists: it's what makes VideoFrameDistortionView's change-detection work
	// again for this GPU-direct source).
	virtual int64_t getDirectFrameIndex() const override;

	// -- IFrameCoupledPoseProvider ----
	virtual bool getLatestFrameCoupledPose(glm::mat4& outTransform, MikanVideoSourceIntrinsics& outIntrinsics,
										   uint32_t& outFrameSeq) const override;

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

	// Pushes the definition's current JBU tuning properties to the live device (a
	// no-op if the device isn't open yet). Called once on device-open (so a freshly
	// opened device picks up the definition's stored values instead of JBUKernel's
	// compiled-in defaults) and again on every live edit to those properties (see
	// onDefinitionMarkedDirty) so runtime adjustments take effect immediately,
	// without a reconnect.
	void pushJBUParamsToDevice();

private:
	IARKitVideoDevicePtr m_arkitVideoDevice= nullptr;
	bool m_bPendingOpen= false;

	// Cached from the most recently received frame bundle - IARKitVideoDevice has no
	// getStreamProperties()-style query the way INetworkVideoDevice does (see
	// NetworkVideoSourceComponent), since ARKit's video dimensions are only known
	// once a bundle actually arrives.
	int m_lastVideoWidth= 0;
	int m_lastVideoHeight= 0;

	// Wrap the plugin's raw GL texture ids (IARKitVideoDevice::getColorTextureGlId/
	// getDepthTextureGlId - see that header's comment on why this crosses the
	// MikanCoreApp/MikanRenderer layering boundary as a raw id rather than an
	// IMkTexturePtr) into IMkExternalTexture wrappers this Editor-side code can
	// hand out via getDirectColorTexture/getDirectDepthTexture. Lazily created on
	// first non-zero id; setExternalPlatformTexture() is cheap to call again every
	// frame after that (it early-outs unless the underlying GL id actually
	// changed - see GlExternalTexture::setExternalPlatformTexture).
	mutable IMkExternalTexturePtr m_colorTextureWrapper;
	mutable IMkExternalTexturePtr m_depthTextureWrapper;

	// Latest frame-coupled pose (ticket E4) - notifyFrameBundleReceived() can fire
	// from a background receiver thread (see that method's own override in the
	// .cpp and IARKitVideoDeviceListener's documented threading contract), while
	// getLatestFrameCoupledPose()/getDirectFrameIndex() are read from the main/GL
	// thread inside CameraComponent::update() - hence the mutex, same pattern as
	// MikanARKitVideoDevice's own "latest depth" cache added in E3.
	mutable std::mutex m_latestPoseMutex;
	struct LatestPose
	{
		glm::mat4 transform= glm::mat4(1.0f);
		MikanVideoSourceIntrinsics intrinsics;
		uint32_t frameSeq= 0;
		bool bValid= false;
	};
	LatestPose m_latestPose;

	// Latest frameSeq seen across ANY bundle (video/depth/pose), not just ones with
	// pose - getDirectFrameIndex() needs this to keep VideoFrameDistortionView's
	// change-detection working even for bundles that arrive without a pose (e.g.
	// depth streaming disabled, or an occasional dropped pose packet). A plain
	// atomic rather than folding into m_latestPoseMutex since it's a single scalar
	// updated unconditionally on every bundle, independent of pose validity.
	std::atomic<int64_t> m_lastBundleFrameSeq{-1};
};
