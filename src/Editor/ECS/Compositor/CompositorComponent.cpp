#include "App.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CameraRequestHandler.h"
#include "ClientSourceManager.h"
#include "CompositorComponent.h"
#include "IMkState.h"
#include "IMkTexture.h"
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
#include "ProjectConfigConstants.h"
#include "SharedTextureWriter.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "TransformComponent.h"
#include "StringUtils.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceComponent.h"
#include "Windows/CompositorNodeEditorWindow.h"

#include "NodeGraphAssetReference.h"
#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>

#include <assert.h>
#include <easy/profiler.h>

// -- CompositorConfig -----
const std::string CompositorDefinition::k_compositorGraphPathPropertyId = "script_path";
const std::string CompositorDefinition::k_cameraIdPropertyId= "camera_id";
const std::string CompositorDefinition::k_ownerStagePropertyId = "owner_stage_id";
const std::string CompositorDefinition::k_spoutEnableOutputNamePropertyId = "spout_enable_output";
const std::string CompositorDefinition::k_spoutOutputNamePropertyId = "spout_output_name";

CompositorDefinition::CompositorDefinition()
	: MikanComponentDefinition()
	, m_compositorId(INVALID_MIKAN_ID)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
	, m_bIsSpoutOutputStreaming(false)
	, m_spoutOutputName(DEFAULT_SPOUT_OUTPUT_NAME)
{
}

CompositorDefinition::CompositorDefinition(
	MikanCompositorID compositorId,
	MikanSceneID ownerSceneId,
	const std::string& compositorName)
	: MikanComponentDefinition(compositorId, compositorName)
	, m_compositorId(compositorId)
	, m_ownerStageId(ownerSceneId)
	, m_nodeGraphAssetRef(std::make_shared<AssetReferenceConfig>())
	, m_bIsSpoutOutputStreaming(false)
	, m_spoutOutputName(DEFAULT_SPOUT_OUTPUT_NAME)
{}

configuru::Config CompositorDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_compositorId;
	pt[k_cameraIdPropertyId] = m_cameraId;
	pt[k_ownerStagePropertyId] = m_ownerStageId;
	pt[k_spoutEnableOutputNamePropertyId] = m_bIsSpoutOutputStreaming;
	pt[k_spoutOutputNamePropertyId] = m_spoutOutputName;

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

	m_cameraId = pt.get_or<int>(k_cameraIdPropertyId, INVALID_MIKAN_ID);
	m_ownerStageId = pt.get_or<int>(k_ownerStagePropertyId, INVALID_MIKAN_ID);
	m_bIsSpoutOutputStreaming = pt.get_or<bool>(k_spoutEnableOutputNamePropertyId, m_bIsSpoutOutputStreaming);
	m_spoutOutputName = pt.get_or<std::string>(k_spoutOutputNamePropertyId, m_spoutOutputName);
	if (m_spoutOutputName.empty())
		m_spoutOutputName = DEFAULT_SPOUT_OUTPUT_NAME;

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
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_cameraIdPropertyId));
	}
}

void CompositorDefinition::setOwnerStageId(MikanSceneID stageId)
{
	if (m_ownerStageId != stageId)
	{
		m_ownerStageId = stageId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_ownerStagePropertyId));
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

void CompositorDefinition::setIsSpoutOutputStreaming(bool bIsStreaming)
{
	if (m_bIsSpoutOutputStreaming != bIsStreaming)
	{
		m_bIsSpoutOutputStreaming = bIsStreaming;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_spoutEnableOutputNamePropertyId));
	}
}

void CompositorDefinition::setSpoutOutputName(const std::string& spoutOutputName)
{
	if (m_spoutOutputName != spoutOutputName)
	{
		m_spoutOutputName = spoutOutputName;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_spoutOutputNamePropertyId));
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

	m_nodeGraphAssetRef = std::make_shared<NodeGraphAssetReference>();
	m_editorFrameBufferTexture = CreateMkTexture();
	m_viewportQuadMesh = createFullscreenQuadMesh(getOwnerWindow(), false);

	// Initialize the compositor graph if we have one assigned
	handleCompositorNodeGraphChanged(getCompositorGraphAssetPath());

	// Listen for changes to the compositor definition
	getCompositorDefinition()->OnMarkedDirty += MakeDelegate(this, &CompositorComponent::onDefinitionChanged);
}

void CompositorComponent::dispose()
{
	stopOutputStreaming();

	getCompositorDefinition()->OnMarkedDirty -= MakeDelegate(this, &CompositorComponent::onDefinitionChanged);

	m_editorFrameBufferTexture = nullptr;
	m_nodeGraph = nullptr;
	m_nodeGraphAssetRef = nullptr;
	m_viewportQuadMesh= nullptr;

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

	const glm::mat4 cameraXform = cameraComponent->getWorldTransform();

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
		auto* clientSourceManager = getOwnerEditorWindow()->getClientSourceManager();
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

			MIKAN_LOG_TRACE("CompositorComponent::update") << "Composite frame " << m_pendingCompositeFrameIndex;
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

			MIKAN_LOG_TRACE("CompositorComponent::update") << "Enqueue frame " << m_lastReadVideoFrameIndex;
			m_frameEventQueue.push(newFrameEvent);
			m_droppedFrameCounter = 0;
		}
		else
		{
			m_droppedFrameCounter++;
			MIKAN_LOG_WARNING("CompositorComponent::update") << "Frame queue overflow. Dropped " << m_droppedFrameCounter << " frames";

			if (m_droppedFrameCounter > 10)
			{
				m_droppedFrameCounter = 0;
				MIKAN_LOG_WARNING("CompositorComponent::update") << "Exceeded dropped frame limit. Flushing frame queue.";

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
		auto* clientSourceManager = getOwnerEditorWindow()->getClientSourceManager();
		for (const std::string& clientSourceId : activeClientSourceIds)
		{
			clientSourceManager->markSourceAsPendingRender(clientSourceId);
		}

		// Track the index of the pending frame
		m_pendingCompositeFrameIndex = newFrameEvent.frame;

		// Tell all clients that we have a new frame to render
		// TODO: Send this event to the camera system instead
		MIKAN_LOG_TRACE("CompositorComponent::update") << "Send frame " << m_pendingCompositeFrameIndex;
		MikanServer::getInstance()->getCameraRequestHandler()->publishCameraNewFrameEvent(newFrameEvent);
	}
}

void CompositorComponent::handleCameraChange(
	CameraComponentPtr oldCameraComponent, 
	CameraComponentPtr newCameraComponent)
{
	VideoSourceComponentPtr oldVideoSourceComponent= oldCameraComponent->getVideoSourceComponent();
	VideoSourceComponentPtr newVideoSourceComponent = newCameraComponent->getVideoSourceComponent();

	if (oldVideoSourceComponent)
	{
		unbindVideoSourceEvents(oldVideoSourceComponent);
	}

	if (newVideoSourceComponent)
	{
		bindVideoSourceEvents(newVideoSourceComponent);
	}

	onVideoFrameSizeChanged(newVideoSourceComponent);
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

void CompositorComponent::disposeVideoBuffers()
{
	m_videoDistortionView = nullptr;
}

void CompositorComponent::allocateVideoBuffers(VideoSourceComponentPtr videoSource)
{
	// Create a distortion view to read the incoming video frames into a texture
	m_videoDistortionView = std::make_shared<VideoFrameDistortionView>(
		getOwnerWindow(),
		videoSource,
		VIDEO_FRAME_HAS_BGR_UNDISTORT_FLAG | VIDEO_FRAME_HAS_GL_TEXTURE_FLAG,
		videoSource->getVideoSourceDefinition()->getVideoFrameQueueSize());

	// Always use the undistorted video frame for compositing
	m_videoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_undistored);
}

void CompositorComponent::disposeCompositingTextures()
{
	m_editorFrameBufferTexture->disposeTexture();
}

void CompositorComponent::createCompositingTextures(int width, int height)
{
	// Also create a texture a for the editor to render to when the editor is active
	m_editorFrameBufferTexture->setSize(width, height);
	m_editorFrameBufferTexture->setTextureFormat(MK_RGBA);
	m_editorFrameBufferTexture->setBufferFormat(MK_RGBA);
	m_editorFrameBufferTexture->setGenerateMipMap(false);
	// ... but don't allocate it create texture until we need it
}

void CompositorComponent::onVideoFrameSizeChanged(VideoSourceComponentPtr videoSource)
{
	disposeCompositingTextures();
	disposeVideoBuffers();

	// Create a frame buffer and texture to do the compositing work in
	int frameWidth, frameHeight;
	if (videoSource->getVideoPixelDimensions(frameWidth, frameHeight))
	{
		createCompositingTextures(frameWidth, frameHeight);
		allocateVideoBuffers(videoSource);
	}
}

void CompositorComponent::updateCompositeFrame()
{
	EASY_FUNCTION();

	assert(m_pendingCompositeFrameIndex != 0);

	// Compute the next undistorted video frame
	m_videoDistortionView->processVideoFrame(m_pendingCompositeFrameIndex);

	// Perform the compositor evaluation if in MainWindow mode
	// (Editor window runs graph evaluation in its own update loop)
	if (m_evaluatorWindow == eCompositorEvaluatorWindow::mainWindow)
	{
		// If we have a valid compositor node graph, use that to composite the frame
		if (m_nodeGraph)
		{
			updateCompositeFrameNodeGraph();
		}
	}

	// Remember the index of the last frame we composited
	m_lastCompositedFrameIndex = m_pendingCompositeFrameIndex;

	// Clear the pending composite frame index
	m_pendingCompositeFrameIndex = 0;

	// Reset the time since the last frame was composited
	m_timeSinceLastFrameComposited = 0.f;

	// Tell any listeners that a new frame was composited
	if (OnNewFrameComposited)
	{
		OnNewFrameComposited();
	}
}

IMkTexturePtr CompositorComponent::getVideoSourceTexture(eVideoTextureSource textureSource) const
{
	switch (textureSource)
	{
	case eVideoTextureSource::video_texture:
		return (m_videoDistortionView != nullptr) ? m_videoDistortionView->getVideoTexture() : IMkTexturePtr();
	case eVideoTextureSource::distortion_texture:
		return (m_videoDistortionView != nullptr) ? m_videoDistortionView->getDistortionTexture() : IMkTexturePtr();
	}

	return IMkTexturePtr();
}

IMkTexturePtr CompositorComponent::getVideoPreviewTexture(eVideoTextureSource textureSource) const
{
	// For now, just return the same texture as the video source
	return getVideoSourceTexture(textureSource);
}

void CompositorComponent::setCompositorEvaluatorWindow(eCompositorEvaluatorWindow evalWindow)
{
	if (m_evaluatorWindow != evalWindow)
	{
		m_editorFrameBufferTexture->disposeTexture();

		if (evalWindow == eCompositorEvaluatorWindow::editorWindow)
		{
			m_editorFrameBufferTexture->createTexture();
		}

		m_evaluatorWindow = evalWindow;
	}
}

IMkTexturePtr CompositorComponent::getEditorWritableFrameTexture() const
{
	return m_editorFrameBufferTexture;
}

IMkTextureConstPtr CompositorComponent::getCompositedFrameTexture() const
{
	switch (m_evaluatorWindow)
	{
	case eCompositorEvaluatorWindow::mainWindow:
		return m_nodeGraph ? m_nodeGraph->getCompositedFrameTexture() : IMkTextureConstPtr();
	case eCompositorEvaluatorWindow::editorWindow:
		return m_editorFrameBufferTexture;
	}

	return IMkTextureConstPtr();
}

void CompositorComponent::editCompositorGraph()
{
	App* app = App::getInstance();

	if (!app->hasWindowOfType<CompositorNodeEditorWindow>())
	{
		app->createAppWindow<CompositorNodeEditorWindow>()
			->bindCompositorComponent(getSelfPtr<CompositorComponent>());
	}
}

void CompositorComponent::addNewCompositorGraph()
{
	removeCompositorGraph();
	editCompositorGraph();
}

void CompositorComponent::removeCompositorGraph()
{
	getCompositorDefinition()->setCompositorGraphPath(std::filesystem::path());
}

void CompositorComponent::updateCompositeFrameNodeGraph()
{
	NodeEvaluator evaluator = {};
	evaluator
		.setCurrentWindow(getOwnerWindow())
		.setDeltaSeconds(m_timeSinceLastFrameComposited);

	if (m_nodeGraph->compositeFrame(evaluator))
	{
		// Publish the composited frame to Spout if streaming is enabled
		if (getIsOutputStreaming())
		{
			IMkTextureConstPtr frameTexture = getCompositedFrameTexture();
		
			if (frameTexture != nullptr && m_renderTargetWriteAccessor->getIsInitialized())
			{
				// TODO: Make this graphics API agnostic
				uint32_t textureId= frameTexture->getGlTextureId();
		
				m_renderTargetWriteAccessor->writeColorFrameTexture(&textureId);
			}
		}
	}
	else
	{
		for (const NodeEvaluationError& error : evaluator.getErrors())
		{
			MIKAN_LOG_ERROR("CompositorComponent::updateCompositeFrame")
				<< "Compositor graph eval error: " << error.errorMessage;
		}
	}
}

void CompositorComponent::renderToViewportQuad() const
{
	IMkTextureConstPtr compositedFrameTexture = getCompositedFrameTexture();
	if (compositedFrameTexture)
	{
		MkMaterialInstancePtr materialInstance = m_viewportQuadMesh->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			// Bind the color texture
			materialInstance->setTextureBySemantic(eUniformSemantic::rgbTexture, compositedFrameTexture);

			// Draw the color texture
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				MkScopedState scopedState = 
					getOwnerWindow()->getMkStateStack().createScopedState("CompositorComponentRender");
				scopedState.getStackState()->disableFlag(eMkStateFlagType::depthTest);

				m_viewportQuadMesh->drawElements();
			}
		}
	}
}

void CompositorComponent::updateOutputStreaming()
{
	CompositorDefinitionConstPtr definition= getCompositorDefinition();
	const bool bWantsOutput = 
		definition->getIsSpoutOutputStreaming() && 
		!definition->getSpoutOutputName().empty();
	const bool bIsStreaming = getIsOutputStreaming();

	// Create the Spout sender if we don't have one already but want to stream
	if (bWantsOutput && !bIsStreaming)
	{
		startOutputStreaming();
	}
	// Stop streaming if we are currently streaming but don't want to anymore
	else if (!bWantsOutput && bIsStreaming)
	{
		stopOutputStreaming();
	}
}

bool CompositorComponent::startOutputStreaming()
{
	// If we are already streaming, do nothing
	if (getIsOutputStreaming())
		return true;

	// Make sure we have a valid texture to stream
	const std::string& spoutOutputName= getCompositorDefinition()->getSpoutOutputName();
	if (spoutOutputName.empty())
		return false;

	IMkTextureConstPtr compositorTexture = getCompositedFrameTexture();
	if (compositorTexture == nullptr)
		return false;

	// Compositing buffer should always be RGBA 32BPP
	// Spout can only support RGBA32 and BGRA32
	assert(compositorTexture->getBufferFormat() == MK_RGBA);

	SharedTextureDescriptor sharedTextureDescriptor;
	sharedTextureDescriptor.color_buffer_type = SharedColorBufferType::RGBA32;
	sharedTextureDescriptor.depth_buffer_type = SharedDepthBufferType::NODEPTH;
	sharedTextureDescriptor.width = compositorTexture->getTextureWidth();
	sharedTextureDescriptor.height = compositorTexture->getTextureHeight();
	sharedTextureDescriptor.graphicsAPI = SharedClientGraphicsApi::OpenGL;

	// Setup render target write accessor
	m_renderTargetWriteAccessor = createSharedTextureWriteAccessor(spoutOutputName);
	m_renderTargetWriteAccessor->initialize(&sharedTextureDescriptor, true, nullptr);

	return true;
}

bool CompositorComponent::getIsOutputStreaming() const
{
	return m_renderTargetWriteAccessor && m_renderTargetWriteAccessor->getIsInitialized();
}

void CompositorComponent::stopOutputStreaming()
{
	if (m_renderTargetWriteAccessor)
	{
		m_renderTargetWriteAccessor->dispose();
		m_renderTargetWriteAccessor = nullptr;
	}
}

bool CompositorComponent::start()
{
	if (getIsRunning())
		return true;

	//TODO: Start the video source?
	m_bIsRunning = true;
	m_timeSinceLastFrameComposited = 0.f;

	// Update the output streaming state in case it changed while we were stopped
	updateOutputStreaming();

	return true;
}

void CompositorComponent::stop()
{
	stopOutputStreaming();

	//TODO: Stop the video source?
	m_bIsRunning = false;
}

MikanStageID CompositorComponent::getOwnerStageId() const
{
	return getCompositorDefinition()->getOwnerStageId();
}

StageComponentPtr CompositorComponent::getOwnerStageComponent() const
{
	return getObjectSystemOfType<StageObjectSystem>()->getStageById(getOwnerStageId());
}

CameraComponentPtr CompositorComponent::getCameraComponent() const
{
	MikanCameraID cameraId = getCompositorDefinition()->getCameraId();

	return getObjectSystemOfType<CameraObjectSystem>()->getCameraById(cameraId);
}

VideoSourceComponentPtr CompositorComponent::getVideoSourceComponent() const
{
	CameraComponentPtr cameraComponent = getCameraComponent();
	if (cameraComponent)
	{
		return cameraComponent->getVideoSourceComponent();
	}

	return VideoSourceComponentPtr();
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

std::filesystem::path CompositorComponent::getCompositorGraphAssetPath() const
{
	return getCompositorDefinition()->getCompositorGraphPath();
}

void CompositorComponent::setCompositorGraphAssetPath(const std::filesystem::path& assetRefPath)
{
	if (getCompositorDefinition()->getCompositorGraphPath() != assetRefPath)
	{
		handleCompositorNodeGraphChanged(assetRefPath);
		getCompositorDefinition()->setCompositorGraphPath(assetRefPath);
	}
}

void CompositorComponent::handleCompositorNodeGraphChanged(const std::filesystem::path& newAssetRefPath)
{
	m_nodeGraphAssetRef->setAssetPath(newAssetRefPath);

	if (m_nodeGraphAssetRef->isValid())
	{
		m_nodeGraph =
			std::dynamic_pointer_cast<CompositorNodeGraph>(
				NodeGraphFactory::loadNodeGraph(getOwnerWindow(), newAssetRefPath));

		if (m_nodeGraph)
		{
			MIKAN_LOG_INFO("CompositorComponent::handleCompositorNodeGraphChanged")
				<< "Loaded compositor graph: " << newAssetRefPath;

			m_nodeGraph->bindToCompositorComponent(getSelfPtr<CompositorComponent>());
		}
		else
		{
			MIKAN_LOG_ERROR("CompositorComponent::handleCompositorNodeGraphChanged")
				<< "Failed to load compositor graph: " << newAssetRefPath;
		}
	}
	else
	{
		m_nodeGraph = nullptr;
	}
}

void CompositorComponent::onDefinitionChanged(CommonConfigPtr configPtr, const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(CompositorDefinition::k_spoutEnableOutputNamePropertyId))
	{
		stopOutputStreaming();
		updateOutputStreaming();
	}
	
	if (changedPropertySet.hasPropertyName(CompositorDefinition::k_spoutOutputNamePropertyId))
	{
		updateOutputStreaming();
	}
}

// -- IRmlPropertyInterface ----
void CompositorComponent::getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlPropertyDescriptors(outDescriptors);

	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			CompositorDefinition::k_cameraIdPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			CompositorDefinition::k_ownerStagePropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			CompositorDefinition::k_compositorGraphPathPropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			CompositorDefinition::k_spoutEnableOutputNamePropertyId));
	outDescriptors.push_back(
		std::make_shared<RmlPropertyDescriptor>(
			CompositorDefinition::k_spoutOutputNamePropertyId));
}

bool CompositorComponent::getPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	Rml::Variant& outValue) const
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == CompositorDefinition::k_cameraIdPropertyId)
	{
		outValue = getCompositorDefinition()->getCameraId();
		return true;
	}
	else if (propertyName == CompositorDefinition::k_ownerStagePropertyId)
	{
		outValue = getCompositorDefinition()->getOwnerStageId();
		return true;
	}
	else if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		outValue = getCompositorDefinition()->getCompositorGraphPath().string();
		return true;
	}
	else if (propertyName == CompositorDefinition::k_spoutEnableOutputNamePropertyId)
	{
		outValue = getCompositorDefinition()->getIsSpoutOutputStreaming();
		return true;
	}
	else if (propertyName == CompositorDefinition::k_spoutOutputNamePropertyId)
	{
		outValue = getCompositorDefinition()->getSpoutOutputName();
		return true;
	}

	return MikanComponent::getPropertyValueFromRml(propertyDesc, outValue);
}

bool CompositorComponent::setPropertyValueFromRml(
	RmlPropertyDescriptorConstPtr propertyDesc,
	const Rml::Variant& inValue)
{
	const std::string& propertyName = propertyDesc->getName();

	if (propertyName == CompositorDefinition::k_cameraIdPropertyId)
	{
		const MikanCameraID cameraId = inValue.Get<int>();
		getCompositorDefinition()->setCameraId(cameraId);
		return true;
	}
	else if (propertyName == CompositorDefinition::k_ownerStagePropertyId)
	{
		const MikanStageID stageId = inValue.Get<int>();
		getCompositorDefinition()->setOwnerStageId(stageId);
		return true;
	}
	else if (propertyName == CompositorDefinition::k_compositorGraphPathPropertyId)
	{
		const Rml::String fileString = inValue.Get<Rml::String>();
		const std::filesystem::path filePath(fileString);

		getCompositorDefinition()->setCompositorGraphPath(filePath);
		return true;
	}
	else if (propertyName == CompositorDefinition::k_spoutEnableOutputNamePropertyId)
	{
		const bool bIsStreaming = inValue.Get<bool>();
		getCompositorDefinition()->setIsSpoutOutputStreaming(bIsStreaming);
		return true;
	}
	else if (propertyName == CompositorDefinition::k_spoutOutputNamePropertyId)
	{
		const Rml::String spoutOutputName = inValue.Get<Rml::String>();
		getCompositorDefinition()->setSpoutOutputName(spoutOutputName);
		return true;
	}

	return MikanComponent::setPropertyValueFromRml(propertyDesc, inValue);
}

// -- IRmlFunctionInterface ----
void CompositorComponent::getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	MikanComponent::getRmlFunctionDescriptors(outDescriptors);
}