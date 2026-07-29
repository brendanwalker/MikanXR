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

ARKitVideoSourceDefinition::ARKitVideoSourceDefinition()
	: VideoSourceDefinition()
	, m_basePort(DEFAULT_ARKIT_BASE_PORT)
{
}

ARKitVideoSourceDefinition::ARKitVideoSourceDefinition(MikanVideoSourceID videoSourceId)
	: VideoSourceDefinition(videoSourceId)
	, m_basePort(DEFAULT_ARKIT_BASE_PORT)
{
}

configuru::Config ARKitVideoSourceDefinition::writeToJSON()
{
	configuru::Config pt= VideoSourceDefinition::writeToJSON();

	pt[k_basePortPropertyId]= m_basePort;

	return pt;
}

void ARKitVideoSourceDefinition::readFromJSON(const configuru::Config& pt)
{
	VideoSourceDefinition::readFromJSON(pt);

	m_basePort= pt.get_or<int>(k_basePortPropertyId, m_basePort);
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
}

bool ARKitVideoSourceComponent::getPropertyValue(const std::string& propertyName, MikanVariant& outValue) const
{
	ARKitVideoSourceDefinitionPtr definition= getARKitVideoSourceDefinition();

	if (propertyName == ARKitVideoSourceDefinition::k_basePortPropertyId)
	{
		outValue= definition->getBasePort();
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

	return VideoSourceComponent::setPropertyValue(propertyName, inValue);
}

void ARKitVideoSourceComponent::onDefinitionMarkedDirty(CommonConfigPtr configPtr,
														const ConfigPropertyChangeSet& changedPropertySet)
{
	// basePort is the only property left that affects the open connection -
	// changing it must force a reconnect.
	if (changedPropertySet.hasPropertyName(ARKitVideoSourceDefinition::k_basePortPropertyId))
	{
		closeVideoSource();
		openVideoSource();
	}
	else
	{
		VideoSourceComponent::onDefinitionMarkedDirty(configPtr, changedPropertySet);
	}
}
