#include "ARKitVideoSourceComponent.h"
#include "ARKitVideoSourceSystem.h"
#include "IARKitVideoDeviceManager.h"
#include "IMkTexture.h"
#include "MikanObject.h"
#include "MikanServer.h"
#include "MikanVideoSourceTypes.h"
#include "ProjectConfigConstants.h"
#include "ThreadUtils.h"
#include "VideoSourceRequestHandler.h"

#include "opencv2/opencv.hpp"

#include <assert.h>

// -- ARKitVideoSourceDefinition -----
const std::string ARKitVideoSourceDefinition::k_basePortPropertyId= "base_port";
const std::string ARKitVideoSourceDefinition::k_depthStreamingEnabledPropertyId= "depth_streaming_enabled";
const std::string ARKitVideoSourceDefinition::k_jbuRadiusPropertyId= "jbu_radius";
const std::string ARKitVideoSourceDefinition::k_jbuSigmaSpatialPropertyId= "jbu_sigma_spatial";
const std::string ARKitVideoSourceDefinition::k_jbuSigmaColorPropertyId= "jbu_sigma_color";
const std::string ARKitVideoSourceDefinition::k_jbuConfWeightLowPropertyId= "jbu_conf_weight_low";
const std::string ARKitVideoSourceDefinition::k_jbuConfWeightMediumPropertyId= "jbu_conf_weight_medium";

// Mirrors JBUParams' own defaults (ticket D5 - see
// MikanARKitVideo/Private/Cuda/JBUKernel.h). Duplicated here as plain literals
// rather than including JBUKernel.h, since that header pulls in <cuda.h> and this
// is Editor ECS code that doesn't otherwise depend on the CUDA Toolkit.
#define DEFAULT_ARKIT_JBU_RADIUS 32
#define DEFAULT_ARKIT_JBU_SIGMA_SPATIAL 16.0f
#define DEFAULT_ARKIT_JBU_SIGMA_COLOR 15.0f
#define DEFAULT_ARKIT_JBU_CONF_WEIGHT_LOW 0.0f
#define DEFAULT_ARKIT_JBU_CONF_WEIGHT_MEDIUM 0.5f

ARKitVideoSourceDefinition::ARKitVideoSourceDefinition()
	: VideoSourceDefinition()
	, m_basePort(DEFAULT_ARKIT_BASE_PORT)
	, m_bDepthStreamingEnabled(true)
	, m_jbuRadius(DEFAULT_ARKIT_JBU_RADIUS)
	, m_jbuSigmaSpatial(DEFAULT_ARKIT_JBU_SIGMA_SPATIAL)
	, m_jbuSigmaColor(DEFAULT_ARKIT_JBU_SIGMA_COLOR)
	, m_jbuConfWeightLow(DEFAULT_ARKIT_JBU_CONF_WEIGHT_LOW)
	, m_jbuConfWeightMedium(DEFAULT_ARKIT_JBU_CONF_WEIGHT_MEDIUM)
{
}

ARKitVideoSourceDefinition::ARKitVideoSourceDefinition(MikanVideoSourceID videoSourceId)
	: VideoSourceDefinition(videoSourceId)
	, m_basePort(DEFAULT_ARKIT_BASE_PORT)
	, m_bDepthStreamingEnabled(true)
	, m_jbuRadius(DEFAULT_ARKIT_JBU_RADIUS)
	, m_jbuSigmaSpatial(DEFAULT_ARKIT_JBU_SIGMA_SPATIAL)
	, m_jbuSigmaColor(DEFAULT_ARKIT_JBU_SIGMA_COLOR)
	, m_jbuConfWeightLow(DEFAULT_ARKIT_JBU_CONF_WEIGHT_LOW)
	, m_jbuConfWeightMedium(DEFAULT_ARKIT_JBU_CONF_WEIGHT_MEDIUM)
{
}

configuru::Config ARKitVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt= VideoSourceDefinition::writeToJSON();

	pt[k_basePortPropertyId]= m_basePort;
	pt[k_depthStreamingEnabledPropertyId]= m_bDepthStreamingEnabled;
	pt[k_jbuRadiusPropertyId]= m_jbuRadius;
	pt[k_jbuSigmaSpatialPropertyId]= m_jbuSigmaSpatial;
	pt[k_jbuSigmaColorPropertyId]= m_jbuSigmaColor;
	pt[k_jbuConfWeightLowPropertyId]= m_jbuConfWeightLow;
	pt[k_jbuConfWeightMediumPropertyId]= m_jbuConfWeightMedium;

	return pt;
}

void ARKitVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);

	m_basePort= pt.get_or<int>(k_basePortPropertyId, m_basePort);
	m_bDepthStreamingEnabled= pt.get_or<bool>(k_depthStreamingEnabledPropertyId, m_bDepthStreamingEnabled);
	m_jbuRadius= pt.get_or<int>(k_jbuRadiusPropertyId, m_jbuRadius);
	m_jbuSigmaSpatial= pt.get_or<float>(k_jbuSigmaSpatialPropertyId, m_jbuSigmaSpatial);
	m_jbuSigmaColor= pt.get_or<float>(k_jbuSigmaColorPropertyId, m_jbuSigmaColor);
	m_jbuConfWeightLow= pt.get_or<float>(k_jbuConfWeightLowPropertyId, m_jbuConfWeightLow);
	m_jbuConfWeightMedium= pt.get_or<float>(k_jbuConfWeightMediumPropertyId, m_jbuConfWeightMedium);
}

bool ARKitVideoSourceDefinition::readFromInitParams(MikanObjectSystem* ownerObjectSystem,
													const Serialization::PolymorphicObjectPtr& initParams)
{
	if (!VideoSourceDefinition::readFromInitParams(ownerObjectSystem, initParams))
		return false;

	const auto* componentValues= initParams.getTypedPointer<MikanARKitVideoSourceValues>();
	if (componentValues)
	{
		m_basePort= componentValues->base_port;
		m_bDepthStreamingEnabled= componentValues->depth_streaming_enabled;
		m_jbuRadius= componentValues->jbu_radius;
		m_jbuSigmaSpatial= componentValues->jbu_sigma_spatial;
		m_jbuSigmaColor= componentValues->jbu_sigma_color;
		m_jbuConfWeightLow= componentValues->jbu_conf_weight_low;
		m_jbuConfWeightMedium= componentValues->jbu_conf_weight_medium;
	}

	return true;
}

void ARKitVideoSourceDefinition::setBasePort(int basePort)
{
	if (basePort != m_basePort)
	{
		m_basePort= basePort;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_basePortPropertyId));
	}
}

void ARKitVideoSourceDefinition::setDepthStreamingEnabled(bool depthStreamingEnabled)
{
	if (depthStreamingEnabled != m_bDepthStreamingEnabled)
	{
		m_bDepthStreamingEnabled= depthStreamingEnabled;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_depthStreamingEnabledPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuRadius(int jbuRadius)
{
	if (jbuRadius != m_jbuRadius)
	{
		m_jbuRadius= jbuRadius;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuRadiusPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuSigmaSpatial(float jbuSigmaSpatial)
{
	if (jbuSigmaSpatial != m_jbuSigmaSpatial)
	{
		m_jbuSigmaSpatial= jbuSigmaSpatial;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuSigmaSpatialPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuSigmaColor(float jbuSigmaColor)
{
	if (jbuSigmaColor != m_jbuSigmaColor)
	{
		m_jbuSigmaColor= jbuSigmaColor;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuSigmaColorPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuConfWeightLow(float jbuConfWeightLow)
{
	if (jbuConfWeightLow != m_jbuConfWeightLow)
	{
		m_jbuConfWeightLow= jbuConfWeightLow;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuConfWeightLowPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuConfWeightMedium(float jbuConfWeightMedium)
{
	if (jbuConfWeightMedium != m_jbuConfWeightMedium)
	{
		m_jbuConfWeightMedium= jbuConfWeightMedium;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuConfWeightMediumPropertyId));
	}
}

// -- ARKitVideoSourceComponent -----
ARKitVideoSourceComponent::ARKitVideoSourceComponent(MikanObjectWeakPtr owner)
	: VideoSourceComponent(owner)
	, m_arkitVideoDevice(nullptr)
{
}

// -- IEntityAccessor ----
rfk::Struct const* ARKitVideoSourceComponent::getClientAPIValuesStructType() const
{
	return &MikanARKitVideoSourceValues::staticGetArchetype();
}

void ARKitVideoSourceComponent::init()
{
	m_bWantsUpdate= true;
	MikanComponent::init();

	// Attempt to open the video source
	openVideoSource();
}

void ARKitVideoSourceComponent::dispose()
{
	// Close the video source if it is open
	closeVideoSource();

	// Call the base class dispose method
	MikanComponent::dispose();
}

void ARKitVideoSourceComponent::setDefinition(MikanComponentDefinitionPtr definition)
{
	MikanComponent::setDefinition(definition);

	// Close any open video source that was open
	closeVideoSource();
}

std::string ARKitVideoSourceComponent::getDevicePath() const
{
	if (m_arkitVideoDevice != nullptr)
	{
		return m_arkitVideoDevice->getDevicePath();
	}

	return "";
}

std::string ARKitVideoSourceComponent::getDeviceAPI() const { return "ARKitVideoSource"; }

bool ARKitVideoSourceComponent::openVideoSource()
{
	// If the video source is already open, do nothing
	if (m_arkitVideoDevice != nullptr)
		return true;

	ARKitVideoSourceDefinitionPtr definition= getARKitVideoSourceDefinition();

	auto arkitVideoSourceSystem= std::static_pointer_cast<ARKitVideoSourceSystem>(getOwnerObject()->getOwnerSystem());
	IARKitVideoDeviceManagerPtr arkitVideoDeviceManager= arkitVideoSourceSystem->getARKitVideoDeviceManager();
	if (!arkitVideoDeviceManager)
	{
		// If the manager is still initializing, mark this component as pending so the
		// system will retry openVideoSource() once the manager is ready
		auto arkitVideoManagerState= arkitVideoSourceSystem->getARKitVideoManagerState();
		if (arkitVideoManagerState == ARKitVideoDeviceManagerLoader::eARKitVideoManagerState::uninitialized
			|| arkitVideoManagerState == ARKitVideoDeviceManagerLoader::eARKitVideoManagerState::initializing)
		{
			m_bPendingOpen= true;
			return true;
		}
		else
		{
			return false;
		}
	}
	m_bPendingOpen= false;

	ARKitVideoConnectionSettings connectionSettings;
	connectionSettings.basePort= static_cast<uint16_t>(definition->getBasePort());
	connectionSettings.depthStreamingEnabled= definition->getDepthStreamingEnabled();

	m_arkitVideoDevice= arkitVideoDeviceManager->createVideoDevice(connectionSettings);
	if (m_arkitVideoDevice == nullptr)
		return false;

	// Listen for events from the ARKit video device before calling open()
	// so that notifyDeviceOpened() is received when the async open completes
	m_arkitVideoDevice->addListener(this);

	// Kick off the async open — post-open setup happens in notifyDeviceOpened()
	if (m_arkitVideoDevice->open() == eVideoOpeningStatus::failed)
	{
		m_arkitVideoDevice->removeListener(this);
		m_arkitVideoDevice= nullptr;
		return false;
	}

	return true;
}

void ARKitVideoSourceComponent::closeVideoSource()
{
	m_bPendingOpen= false;

	if (m_arkitVideoDevice != nullptr)
	{
		// Remove the listener for the ARKit video device
		m_arkitVideoDevice->removeListener(this);

		// Stop the video stream if it is running
		forceStopVideoStream();

		// Close the ARKit video device
		m_arkitVideoDevice->close();

		// Tell the ARKit video device manager to destroy the device
		auto arkitVideoSourceSystem=
			std::static_pointer_cast<ARKitVideoSourceSystem>(getOwnerObject()->getOwnerSystem());
		IARKitVideoDeviceManagerPtr arkitVideoDeviceManager= arkitVideoSourceSystem->getARKitVideoDeviceManager();
		if (arkitVideoDeviceManager)
		{
			arkitVideoDeviceManager->destroyVideoDevice(m_arkitVideoDevice);
		}

		// Clear the ARKit video device pointer
		m_arkitVideoDevice= nullptr;
	}

	m_lastVideoWidth= 0;
	m_lastVideoHeight= 0;

	// Let any connected clients know that the video source closed
	MikanServer::getInstance()->getVideoSourceRequestHandler()->publishVideoSourceClosedEvent();
	if (OnClosed)
	{
		OnClosed(getSelfPtr<VideoSourceComponent>());
	}
}

eVideoStreamingStatus ARKitVideoSourceComponent::startVideoStreamInternal()
{
	if (m_arkitVideoDevice != nullptr)
	{
		eVideoStreamingStatus status= m_arkitVideoDevice->startVideoStream();
		if (OnStarted && (status == eVideoStreamingStatus::started || status == eVideoStreamingStatus::pendingStart))
		{
			OnStarted(getSelfPtr<VideoSourceComponent>());
		}

		return status;
	}

	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus ARKitVideoSourceComponent::getVideoStreamingStatus() const
{
	if (m_arkitVideoDevice != nullptr)
	{
		return m_arkitVideoDevice->getVideoStreamingStatus();
	}

	return eVideoStreamingStatus::failed;
}

void ARKitVideoSourceComponent::stopVideoStreamInternal()
{
	if (m_arkitVideoDevice != nullptr)
	{
		m_arkitVideoDevice->stopVideoStream();

		if (OnStopped)
		{
			OnStopped(getSelfPtr<VideoSourceComponent>());
		}
	}
}

bool ARKitVideoSourceComponent::getVideoPixelDimensions(int& outPixelWidth, int& outPixelHeight) const
{
	if (m_lastVideoWidth > 0 && m_lastVideoHeight > 0)
	{
		outPixelWidth= m_lastVideoWidth;
		outPixelHeight= m_lastVideoHeight;
		return true;
	}

	return false;
}

// -- Zero-copy GPU texture access (ticket E3) ----
IMkTexturePtr ARKitVideoSourceComponent::getDirectColorTexture() const
{
	if (m_arkitVideoDevice == nullptr)
		return IMkTexturePtr();

	uint32_t glId= m_arkitVideoDevice->getColorTextureGlId();
	if (glId == 0)
		return IMkTexturePtr();

	if (m_colorTextureWrapper == nullptr)
		m_colorTextureWrapper= CreateMkExternalTexture();

	m_colorTextureWrapper->setExternalPlatformTexture(&glId);
	return m_colorTextureWrapper;
}

IMkTexturePtr ARKitVideoSourceComponent::getDirectDepthTexture() const
{
	if (m_arkitVideoDevice == nullptr)
		return IMkTexturePtr();

	uint32_t glId= m_arkitVideoDevice->getDepthTextureGlId();
	if (glId == 0)
		return IMkTexturePtr();

	if (m_depthTextureWrapper == nullptr)
		m_depthTextureWrapper= CreateMkExternalTexture();

	m_depthTextureWrapper->setExternalPlatformTexture(&glId);
	return m_depthTextureWrapper;
}

// -- IARKitVideoDeviceListener ----
void ARKitVideoSourceComponent::notifyDeviceOpened(const IARKitVideoDevice* device)
{
	if (device != m_arkitVideoDevice.get())
		return;

	// Let any connected clients know that the video source opened
	MikanServer::getInstance()->getVideoSourceRequestHandler()->publishVideoSourceOpenedEvent();
	if (OnOpened)
	{
		OnOpened(getSelfPtr<VideoSourceComponent>());
	}
}

void ARKitVideoSourceComponent::notifyDeviceClosed(const IARKitVideoDevice* device)
{
	if (device == m_arkitVideoDevice.get())
	{
		// The video source is now invalidated, so we can clear the pointer
		// but we still want to clean up the video source state
		m_arkitVideoDevice= nullptr;
		closeVideoSource();
	}
}

void ARKitVideoSourceComponent::notifyFrameBundleReceived(const ARKitVideoFrameBundle& bundle)
{
	assert(m_arkitVideoDevice != nullptr);

	// As of ticket C4, the video pipeline hardware-decodes straight to CUDA device
	// memory (see MikanARKitVideoDevice::notifyFrameBundleReceived) - bundle.videoData
	// is always null today, so there's nothing to hand to writeVideoFrame() yet.
	// This guard is the natural, forward-compatible consumer of the documented
	// ARKitVideoFrameBundle contract ("videoData ... valid only during the
	// callback") rather than dead code: once a later ticket gives the ARKit pipeline
	// a CPU-readable (or CUDA-GL composited) preview path, this starts writing real
	// frames with no further change needed here.
	if (bundle.hasVideo && bundle.videoData != nullptr)
	{
		ARKitVideoSourceDefinitionPtr definition= getARKitVideoSourceDefinition();
		const bool is_frame_flipped= definition->getIsFrameMirrored();
		cv::Size bufferDimensions(bundle.videoWidth, bundle.videoHeight);

		m_lastVideoWidth= bundle.videoWidth;
		m_lastVideoHeight= bundle.videoHeight;

		writeVideoFrame(bundle.videoData, bufferDimensions, is_frame_flipped);
	}

	// bundle.depth is deliberately left untouched here - there's no depth-texture
	// slot to write into yet (see ticket E3, which owns adding one).
}

// -- IPropertyInterface ----
void ARKitVideoSourceComponent::getPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	VideoSourceComponent::getPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(ARKitVideoSourceDefinition::k_basePortPropertyId, MikanVariantType::INT)
			->setDefaultValue(DEFAULT_ARKIT_BASE_PORT));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
								 ARKitVideoSourceDefinition::k_depthStreamingEnabledPropertyId, MikanVariantType::BOOL)
								 ->setDefaultValue(true));
	outDescriptors.push_back(
		std::make_shared<PropertyDescriptor>(ARKitVideoSourceDefinition::k_jbuRadiusPropertyId, MikanVariantType::INT)
			->setDefaultValue(DEFAULT_ARKIT_JBU_RADIUS));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
								 ARKitVideoSourceDefinition::k_jbuSigmaSpatialPropertyId, MikanVariantType::FLOAT)
								 ->setDefaultValue(DEFAULT_ARKIT_JBU_SIGMA_SPATIAL));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(ARKitVideoSourceDefinition::k_jbuSigmaColorPropertyId,
																  MikanVariantType::FLOAT)
								 ->setDefaultValue(DEFAULT_ARKIT_JBU_SIGMA_COLOR));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
								 ARKitVideoSourceDefinition::k_jbuConfWeightLowPropertyId, MikanVariantType::FLOAT)
								 ->setDefaultValue(DEFAULT_ARKIT_JBU_CONF_WEIGHT_LOW));
	outDescriptors.push_back(std::make_shared<PropertyDescriptor>(
								 ARKitVideoSourceDefinition::k_jbuConfWeightMediumPropertyId, MikanVariantType::FLOAT)
								 ->setDefaultValue(DEFAULT_ARKIT_JBU_CONF_WEIGHT_MEDIUM));
}

bool ARKitVideoSourceComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	ARKitVideoSourceDefinitionPtr definition= getARKitVideoSourceDefinition();

	if (propertyName == ARKitVideoSourceDefinition::k_basePortPropertyId)
	{
		outValue= definition->getBasePort();
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_depthStreamingEnabledPropertyId)
	{
		outValue= definition->getDepthStreamingEnabled();
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuRadiusPropertyId)
	{
		outValue= definition->getJbuRadius();
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuSigmaSpatialPropertyId)
	{
		outValue= definition->getJbuSigmaSpatial();
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuSigmaColorPropertyId)
	{
		outValue= definition->getJbuSigmaColor();
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuConfWeightLowPropertyId)
	{
		outValue= definition->getJbuConfWeightLow();
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuConfWeightMediumPropertyId)
	{
		outValue= definition->getJbuConfWeightMedium();
		return true;
	}

	return VideoSourceComponent::getPropertyValue(propertyName, outValue);
}

bool ARKitVideoSourceComponent::setPropertyValue(const std::string& propertyName, const MikanVariant& inValue)
{
	ARKitVideoSourceDefinitionPtr definition= getARKitVideoSourceDefinition();

	if (propertyName == ARKitVideoSourceDefinition::k_basePortPropertyId)
	{
		definition->setBasePort(inValue.getIntValue());
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_depthStreamingEnabledPropertyId)
	{
		definition->setDepthStreamingEnabled(inValue.getBoolValue());
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuRadiusPropertyId)
	{
		definition->setJbuRadius(inValue.getIntValue());
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuSigmaSpatialPropertyId)
	{
		definition->setJbuSigmaSpatial(inValue.getFloatValue());
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuSigmaColorPropertyId)
	{
		definition->setJbuSigmaColor(inValue.getFloatValue());
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuConfWeightLowPropertyId)
	{
		definition->setJbuConfWeightLow(inValue.getFloatValue());
		return true;
	}
	else if (propertyName == ARKitVideoSourceDefinition::k_jbuConfWeightMediumPropertyId)
	{
		definition->setJbuConfWeightMedium(inValue.getFloatValue());
		return true;
	}

	return VideoSourceComponent::setPropertyValue(propertyName, inValue);
}

void ARKitVideoSourceComponent::onDefinitionMarkedDirty(CommonConfigPtr configPtr,
														const ConfigPropertyChangeSet& changedPropertySet)
{
	// Only basePort/depthStreamingEnabled affect the open connection - the JBU
	// tuning params aren't consumed by open()/the live pipeline yet (see their
	// declaration comment on ARKitVideoSourceDefinition), so changing them must not
	// force a reconnect.
	if (changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_basePortPropertyId)
		|| changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_depthStreamingEnabledPropertyId))
	{
		closeVideoSource();
		openVideoSource();
	}
	else
	{
		VideoSourceComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);
	}
}
