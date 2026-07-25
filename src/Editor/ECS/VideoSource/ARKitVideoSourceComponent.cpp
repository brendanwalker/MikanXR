#include "ARKitVideoSourceComponent.h"
#include "ARKitVideoSourceSystem.h"
#include "CameraMath.h"
#include "IARKitVideoDeviceManager.h"
#include "IMkTexture.h"
#include "MikanObject.h"
#include "MikanServer.h"
#include "MikanVideoSourceTypes.h"
#include "ProjectConfigConstants.h"
#include "ThreadUtils.h"
#include "VideoSourceRequestHandler.h"

#include "opencv2/opencv.hpp"

#include <algorithm>
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
	// Clamped rather than left unbounded: the kernel itself tolerates any value
	// (window bounds are clamped internally, see JBUKernel.cu), but an extreme
	// radius (e.g. from a bad remote-client property set) would tank frame time by
	// scanning a huge fraction of the low-res depth plane per output pixel.
	jbuRadius= std::clamp(jbuRadius, 1, 128);
	if (jbuRadius != m_jbuRadius)
	{
		m_jbuRadius= jbuRadius;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuRadiusPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuSigmaSpatial(float jbuSigmaSpatial)
{
	// Clamped above 0 - the kernel computes 1/(2*sigma^2) (see JBUKernel.cpp), so an
	// exact 0 would divide-by-zero and propagate NaN into the upsampled depth output.
	jbuSigmaSpatial= std::clamp(jbuSigmaSpatial, 0.1f, 128.0f);
	if (jbuSigmaSpatial != m_jbuSigmaSpatial)
	{
		m_jbuSigmaSpatial= jbuSigmaSpatial;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuSigmaSpatialPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuSigmaColor(float jbuSigmaColor)
{
	// Same divide-by-zero rationale as setJbuSigmaSpatial above.
	jbuSigmaColor= std::clamp(jbuSigmaColor, 0.1f, 255.0f);
	if (jbuSigmaColor != m_jbuSigmaColor)
	{
		m_jbuSigmaColor= jbuSigmaColor;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuSigmaColorPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuConfWeightLow(float jbuConfWeightLow)
{
	jbuConfWeightLow= std::clamp(jbuConfWeightLow, 0.0f, 1.0f);
	if (jbuConfWeightLow != m_jbuConfWeightLow)
	{
		m_jbuConfWeightLow= jbuConfWeightLow;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_jbuConfWeightLowPropertyId));
	}
}

void ARKitVideoSourceDefinition::setJbuConfWeightMedium(float jbuConfWeightMedium)
{
	jbuConfWeightMedium= std::clamp(jbuConfWeightMedium, 0.0f, 1.0f);
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

	// Apply the definition's stored JBU tuning to the freshly opened device - without
	// this it would silently run with JBUKernel.h's compiled-in defaults instead of
	// whatever the user has configured (or previously tuned) for this source.
	pushJBUParamsToDevice();

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

	// Tracked unconditionally (independent of which fields this particular bundle
	// carries) so getDirectFrameIndex() reflects the latest frameSeq seen at all,
	// not just ones with a pose - see that method's declaration comment.
	m_lastBundleFrameSeq.store(static_cast<int64_t>(bundle.frameSeq));

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

	// bundle.depth is deliberately left untouched here - MikanARKitVideoDevice's own
	// CUDA-GL depth pipeline (ticket E3) reads depth via its own "latest depth"
	// cache, decoupled from this correlator bundle entirely - see that class.

	// Frame-coupled pose (ticket E4).
	if (bundle.hasPose)
	{
		// ARKitPoseFrameBuffer::transform is row-major camera-to-world
		// (IARKitVideoDevice.h) - glm::mat4's constructor/subscript operators are
		// column-major, so this must be transposed, not just memcpy'd. This exact
		// row/column-major mixup was flagged as the single most likely bug spot
		// back when the wire format was first designed (Track A5) - get it wrong
		// here and the camera pose will look like a scrambled/sheared transform,
		// not an obviously-wrong one.
		const float* t= bundle.pose.transform;
		const glm::mat4 cameraToWorld(t[0], t[4], t[8], t[12], t[1], t[5], t[9], t[13], t[2], t[6], t[10], t[14], t[3],
									  t[7], t[11], t[15]);

		MikanVideoSourceIntrinsics intrinsics;
		MikanMonoIntrinsics monoIntrinsics;
		createDefautMonoIntrinsics(static_cast<int>(bundle.pose.imageWidth), static_cast<int>(bundle.pose.imageHeight),
								   monoIntrinsics);
		// Overwrite the synthetic fx/fy/cx/cy with ARKit's real, per-frame values -
		// znear/zfar/pixel_width/pixel_height from the synthetic defaults are kept
		// as-is (ARKit doesn't report clip planes, and pixel dimensions already
		// match since they were used to build the synthetic intrinsics above).
		// ARKit reports no distortion coefficients, so distorted == undistorted.
		monoIntrinsics.undistorted_camera_matrix.x0= bundle.pose.fx;
		monoIntrinsics.undistorted_camera_matrix.y1= bundle.pose.fy;
		monoIntrinsics.undistorted_camera_matrix.x2= bundle.pose.cx;
		monoIntrinsics.undistorted_camera_matrix.y2= bundle.pose.cy;
		monoIntrinsics.distorted_camera_matrix= monoIntrinsics.undistorted_camera_matrix;
		intrinsics.makeMonoIntrinsics()= monoIntrinsics;

		std::lock_guard<std::mutex> lock(m_latestPoseMutex);
		m_latestPose.transform= cameraToWorld;
		m_latestPose.intrinsics= intrinsics;
		m_latestPose.frameSeq= bundle.frameSeq;
		m_latestPose.bValid= true;
	}
}

int64_t ARKitVideoSourceComponent::getDirectFrameIndex() const { return m_lastBundleFrameSeq.load(); }

bool ARKitVideoSourceComponent::getLatestFrameCoupledPose(glm::mat4& outTransform,
														  MikanVideoSourceIntrinsics& outIntrinsics,
														  uint32_t& outFrameSeq) const
{
	std::lock_guard<std::mutex> lock(m_latestPoseMutex);
	if (!m_latestPose.bValid)
		return false;

	outTransform= m_latestPose.transform;
	outIntrinsics= m_latestPose.intrinsics;
	outFrameSeq= m_latestPose.frameSeq;
	return true;
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
	// Only basePort/depthStreamingEnabled affect the open connection - changing them
	// must force a reconnect.
	if (changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_basePortPropertyId)
		|| changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_depthStreamingEnabledPropertyId))
	{
		closeVideoSource();
		openVideoSource();
	}
	else
	{
		// JBU tuning params live-apply to the running device with no reconnect
		// needed - lets the user sweep values in real time while watching the depth
		// preview (AppStage_VideoSourceSettings) for flicker/edge quality.
		if (changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_jbuRadiusPropertyId)
			|| changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_jbuSigmaSpatialPropertyId)
			|| changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_jbuSigmaColorPropertyId)
			|| changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_jbuConfWeightLowPropertyId)
			|| changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_jbuConfWeightMediumPropertyId))
		{
			pushJBUParamsToDevice();
		}

		VideoSourceComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);
	}
}

void ARKitVideoSourceComponent::pushJBUParamsToDevice()
{
	if (m_arkitVideoDevice == nullptr)
		return;

	ARKitVideoSourceDefinitionPtr definition= getARKitVideoSourceDefinition();

	ARKitJBUParams params;
	params.radius= definition->getJbuRadius();
	params.sigmaSpatial= definition->getJbuSigmaSpatial();
	params.sigmaColor= definition->getJbuSigmaColor();
	params.confWeightLow= definition->getJbuConfWeightLow();
	params.confWeightMedium= definition->getJbuConfWeightMedium();

	m_arkitVideoDevice->setJBUParams(params);
}
