#include "App.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CameraRequestHandler.h"
#include "ClientSourceManager.h"
#include "CompositorComponent.h"
#include "IMkState.h"
#include "IMkTriangulatedMesh.h"
#include "Logger.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanServer.h"
#include "MkMaterialInstance.h"
#include "MkScopedState.h"
#include "MkStateStack.h"
#include "ProjectConfig.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "TransformComponent.h"
#include "StringUtils.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceComponent.h"

#include "NodeGraphAssetReference.h"
#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <easy/profiler.h>

// -- CompositorConfig -----
const std::string CompositorDefinition::k_compositorGraphPathPropertyId = "script_path";
const std::string CompositorDefinition::k_cameraPropertyId= "camera_id";
const std::string CompositorDefinition::k_ownerScenePropertyId = "owner_scene_id";

CompositorDefinition::CompositorDefinition()
	: MikanComponentDefinition()
	, m_compositorId(INVALID_MIKAN_ID)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
{
}

CompositorDefinition::CompositorDefinition(
	MikanCompositorID compositorId,
	MikanSceneID ownerSceneId,
	const std::string& compositorName)
	: MikanComponentDefinition(compositorName)
	, m_compositorId(compositorId)
	, m_ownerSceneId(ownerSceneId)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
{}

configuru::Config CompositorDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_compositorId;
	pt[k_cameraPropertyId] = m_cameraId;
	pt[k_ownerScenePropertyId] = m_ownerSceneId;

	if (m_nodeGraphAssetRef)
	{
		pt[k_compositorGraphPathPropertyId] = m_nodeGraphAssetRef->writeToJSON();
	}

	return pt;
}

void CompositorDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_compositorId = pt.get<int>("id");
	m_cameraId = pt.get_or<int>(k_cameraPropertyId, INVALID_MIKAN_ID);
	m_ownerSceneId = pt.get_or<int>(k_ownerScenePropertyId, INVALID_MIKAN_ID);

	m_componentScriptAssetRefConfig = NodeGraphAssetReferenceFactory().allocateAssetReferenceConfig();
	if (pt.has_key(k_compositorGraphPathPropertyId))
	{
		m_componentScriptAssetRefConfig->readFromJSON(pt[k_compositorGraphPathPropertyId]);
	}
}

void CompositorDefinition::setCameraId(MikanCameraID cameraId)
{
	if (m_cameraId != cameraId)
	{
		m_cameraId = cameraId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_cameraPropertyId));
	}
}

void CompositorDefinition::setOwnerSceneId(MikanSceneID sceneId)
{
	if (m_ownerSceneId != sceneId)
	{
		m_ownerSceneId = sceneId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_ownerScenePropertyId));
	}
}

bool CompositorDefinition::hasCompositorGraphPath() const
{
	return !m_nodeGraphAssetRef->assetPath.empty();
}

std::filesystem::path CompositorDefinition::getCompositorGraphPath() const
{
	return m_nodeGraphAssetRef->assetPath;
}

void CompositorDefinition::setCompositorGraphPath(const std::filesystem::path& graphPath)
{
	if (graphPath != m_nodeGraphAssetRef->assetPath)
	{
		m_nodeGraphAssetRef->assetPath= graphPath.string();
		markDirty(ConfigPropertyChangeSet().addPropertyName(MikanComponentDefinition::k_componentScriptPathPropertyId));
	}
}

// -- CompositorComponent -----
CompositorComponent::CompositorComponent(MikanObjectWeakPtr owner)
	: MikanComponent(owner)
{
	m_bWantsUpdate = true;
}

void CompositorComponent::init()
{
	MikanComponent::init();

	m_layerQuadMesh = createFullscreenQuadMesh(getOwnerWindow(), false);
}

void CompositorComponent::dispose()
{
	m_layerQuadMesh= nullptr;

	MikanComponent::dispose();
}

void CompositorComponent::update(float deltaSeconds)
{
	EASY_FUNCTION();

	if (!getIsRunning())
		return;

	CameraComponentPtr cameraComponent= getCameraComponent();
	if (!cameraComponent)
		return;

	const glm::mat4 cameraXform= cameraComponent->getWorldTransform();

	// Keep track of how long it's been since the last frame has been composited
	// This is used to update the timer in compositorNodeGraph
	m_timeSinceLastFrameComposited += deltaSeconds;

	// Composite the next frame if we got all the renders back from the clients
	std::set<std::string> activeClientSourceIds;
	if (m_pendingCompositeFrameIndex != 0 && m_nodeGraph)
	{
		// Gather all client source IDs that are referenced by the node graph
		m_nodeGraph->gatherAllReferencedClientSourceIDs(activeClientSourceIds);

		// See if all client render targets have been updated
		auto* clientSourceManager = ClientSourceManager::getInstance();
		size_t clientSourceReadyCount = 0;
		for (const std::string& clientSourceId : activeClientSourceIds)
		{
			// If the client source is not registered, register it
			if (!clientSourceManager->getIsSourcePendingRender(clientSourceId))
			{
				clientSourceReadyCount++;
			}
		}
		
		// If the video frame and client sources are fresh, composite them together
		if (clientSourceReadyCount == activeClientSourceIds.size())
		{
			// Pop the frame event from the queue now that we are compositing it
			assert(m_frameEventQueue.front().frame == m_pendingCompositeFrameIndex);
			m_frameEventQueue.pop();

			MIKAN_LOG_TRACE("GlFrameCompositor::update") << "Composite frame " << m_pendingCompositeFrameIndex;
			updateCompositeFrame();
		}
	}

	// Fetch new video frames if the video frame queue isn't full
	if (m_videoDistortionView != nullptr && m_videoDistortionView->hasNewVideoFrame())
	{
		// If the queue is full, drop all queued frames to catch up
		if (m_frameEventQueue.size() < m_videoDistortionView->getMaxFrameQueueSize())
		{
			m_lastReadVideoFrameIndex = m_videoDistortionView->readNextVideoFrame();

			MikanCameraNewFrameEvent newFrameEvent;
			newFrameEvent.frame = m_lastReadVideoFrameIndex;

			const glm::vec3 cameraUp(cameraXform[1]); // Camera up is along the y-axis
			const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
			const glm::vec3 cameraPosition(cameraXform[3]); // Camera up is along the y-axis
			newFrameEvent.cameraForward = glm_vec3_to_MikanVector3f(cameraForward);
			newFrameEvent.cameraUp = glm_vec3_to_MikanVector3f(cameraUp);
			newFrameEvent.cameraPosition = glm_vec3_to_MikanVector3f(cameraPosition);

			MIKAN_LOG_TRACE("GlFrameCompositor::update") << "Enqueue frame " << m_lastReadVideoFrameIndex;
			m_frameEventQueue.push(newFrameEvent);
			m_droppedFrameCounter = 0;
		}
		else
		{
			m_droppedFrameCounter++;
			MIKAN_LOG_WARNING("GlFrameCompositor::update") << "Frame queue overflow. Dropped " << m_droppedFrameCounter << " frames";

			if (m_droppedFrameCounter > 10)
			{
				m_droppedFrameCounter = 0;
				MIKAN_LOG_WARNING("GlFrameCompositor::update") << "Exceeded dropped frame limit. Flushing frame queue.";

				while (m_frameEventQueue.size() > 0)
				{
					m_frameEventQueue.pop();
				}
				m_pendingCompositeFrameIndex = 0;
			}
		}
	}

	// If we don't have a pending frame to composite and have a queued frame,
	// the send off the next frame to the clients to render
	if (m_pendingCompositeFrameIndex == 0 && m_frameEventQueue.size() > 0)
	{
		// Grab the next frame event off the queue
		MikanCameraNewFrameEvent newFrameEvent = m_frameEventQueue.front();

		// Mark all client sources as pending
		auto* clientSourceManager = ClientSourceManager::getInstance();
		for (const std::string& clientSourceId : activeClientSourceIds)
		{
			clientSourceManager->markSourceAsPendingRender(clientSourceId);
		}

		// Track the index of the pending frame
		m_pendingCompositeFrameIndex = newFrameEvent.frame;

		// Tell all clients that we have a new frame to render
		// TODO: Send this event to the camera system instead
		MIKAN_LOG_TRACE("GlFrameCompositor::update") << "Send frame " << m_pendingCompositeFrameIndex;
		MikanServer::getInstance()->getCameraRequestHandler()->publishCameraNewFrameEvent(newFrameEvent);
	}
}

void CompositorComponent::handleCameraChange(
	CameraComponentPtr oldCameraComponent, 
	CameraComponentPtr newCameraComponent)
{
	if (oldCameraComponent)
	{
		unbindVideoSourceEvents(oldCameraComponent->getVideoSourceComponent());
	}

	if (newCameraComponent)
	{
		bindVideoSourceEvents(newCameraComponent->getVideoSourceComponent());
	}
}

void CompositorComponent::unbindVideoSourceEvents(VideoSourceComponentPtr videoSource)
{
	if (videoSource != nullptr)
	{
		videoSource->OnFrameSizeChanged 
			-= MakeDelegate(this, &CompositorComponent::onVideoFrameSizeChanged);
	}
}

void CompositorComponent::bindVideoSourceEvents(VideoSourceComponentPtr videoSource)
{
	if (videoSource != nullptr)
	{
		videoSource->OnFrameSizeChanged 
			+= MakeDelegate(this, &CompositorComponent::onVideoFrameSizeChanged);
	}
}

void CompositorComponent::onVideoFrameSizeChanged(const VideoSourceComponent* videoSource)
{

}

void CompositorComponent::updateCompositeFrame()
{

}

void CompositorComponent::render() const
{
	IMkTextureConstPtr compositedFrameTexture = getCompositedFrameTexture();
	if (compositedFrameTexture)
	{
		MkMaterialInstancePtr materialInstance = m_layerQuadMesh->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			// Bind the color texture
			materialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, compositedFrameTexture);

			// Draw the color texture
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				MkScopedState scopedState = 
					getOwnerWindow()->getMkStateStack().createScopedState("GlFrameCompositorRender");
				scopedState.getStackState()->disableFlag(eMkStateFlagType::depthTest);

				m_layerQuadMesh->drawElements();
			}
		}
	}

}

bool CompositorComponent::getIsRunning() const
{
	SceneComponentPtr ownerScene= getOwnerSceneComponent();
	if (ownerScene)
	{
		MikanCompositorID selfCompositorId = getCompositorDefinition()->getCompositorId();
		MikanCompositorID outputCompositorId = ownerScene->getSceneComponentDefinition()->getOutputCompositorId();

		return (outputCompositorId == selfCompositorId);
	}

	return false;
}

SceneComponentPtr CompositorComponent::getOwnerSceneComponent() const
{
	MikanSceneID ownerSceneId= getCompositorDefinition()->getOwnerSceneId();

	return SceneObjectSystem::getSystem()->getSceneById(ownerSceneId);
}

CameraComponentPtr CompositorComponent::getCameraComponent() const
{
	MikanCameraID cameraId = getCompositorDefinition()->getCameraId();

	return CameraObjectSystem::getSystem()->getCameraById(cameraId);
}

void CompositorComponent::setCameraComponent(CameraComponentPtr newCameraComponent)
{
	CameraComponentPtr oldCameraComponent = getCameraComponent();
	MikanCameraID oldCameraId = oldCameraComponent ? oldCameraComponent->getCameraId() : INVALID_MIKAN_ID;
	MikanCameraID newCameraId = newCameraComponent ? newCameraComponent->getCameraId() : INVALID_MIKAN_ID;

	if (newCameraId != oldCameraId)
	{
		// Rebuild the compositor state since the camera has changed
		handleCameraChange(oldCameraComponent, newCameraComponent);

		// Update the camera ID in the compositor definition
		getCompositorDefinition()->setCameraId(newCameraId);
	}
}

IMkTextureConstPtr CompositorComponent::getCompositedFrameTexture() const
{
	return m_nodeGraph ? m_nodeGraph->getCompositedFrameTexture() : IMkTextureConstPtr();
}

// -- IPropertyInterface ----
void CompositorComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	MikanComponent::getPropertyNames(outPropertyNames);

	outPropertyNames.push_back(CompositorDefinition::k_compositorGraphPathPropertyId);
}

bool CompositorComponent::getPropertyDescriptor(const std::string& propertyName, PropertyDescriptor& outDescriptor) const
{
	if (MikanComponent::getPropertyDescriptor(propertyName, outDescriptor))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		outDescriptor = {CompositorDefinition::k_compositorGraphPathPropertyId, ePropertyDataType::datatype_string, ePropertySemantic::filename};
		return true;
	}

	return false;
}

bool CompositorComponent::getPropertyValue(const std::string& propertyName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyValue(propertyName, outValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		Rml::String filepath = getCompositorDefinition()->getCompositorGraphPath().string();

		outValue = filepath;
		return true;
	}

	return false;
}

bool CompositorComponent::getPropertyAttribute(const std::string& propertyName, const std::string& attributeName, Rml::Variant& outValue) const
{
	if (MikanComponent::getPropertyAttribute(propertyName, attributeName, outValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		if (attributeName == *k_PropertyAttributeFileBrowseTitle)
		{
			outValue = "Select a graph";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilter)
		{
			outValue = ".graph";
		}
		else if (attributeName == *k_PropertyAttributeFileBrowseFilterDesc)
		{
			outValue = "Graph Files (.graph)";
		}
	}

	return false;
}

bool CompositorComponent::setPropertyValue(const std::string& propertyName, const Rml::Variant& inValue)
{
	if (MikanComponent::setPropertyValue(propertyName, inValue))
		return true;

	if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		const Rml::String fileString = inValue.Get<Rml::String>();
		const std::filesystem::path filePath(fileString);

		getCompositorDefinition()->setCompositorGraphPath(filePath);
		return true;
	}

	return false;
}