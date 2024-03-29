#include "App.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlFrameCompositor.h"
#include "GlMaterial.h"
#include "GlTexture.h"
#include "GlTextRenderer.h"
#include "GlProgramConfig.h"
#include "InterprocessRenderTargetReader.h"
#include "IGlWindow.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "MainWindow.h"
#include "ModelStencilComponent.h"
#include "PathUtils.h"
#include "ProfileConfig.h"
#include "StringUtils.h"
#include "GlShaderCache.h"
#include "GlStateStack.h"
#include "GlProgram.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlTriangulatedMesh.h"
#include "GlVertexDefinition.h"
#include "VideoSourceManager.h"
#include "VideoSourceView.h"
#include "VideoFrameDistortionView.h"
#include "VRDeviceManager.h"

#include "NodeGraphAssetReference.h"
#include "Graphs/CompositorNodeGraph.h"
#include "Graphs/NodeEvaluator.h"

#include <algorithm>

#include <easy/profiler.h>

// -- GlFrameCompositor ------
GlFrameCompositor* GlFrameCompositor::m_instance= nullptr;

GlFrameCompositor::GlFrameCompositor()
{
	m_config= std::make_shared<GlFrameCompositorConfig>();
	m_instance= this;
	m_nodeGraphAssetRef = std::make_shared<NodeGraphAssetReference>();
}

GlFrameCompositor::~GlFrameCompositor()
{
	m_config= nullptr;
	m_instance = nullptr;
}

bool GlFrameCompositor::startup(IGlWindow* ownerWindow)
{
	EASY_FUNCTION();

	m_ownerWindow= ownerWindow;

	reloadAllCompositorPresets();

	m_rgbFrameShader = GlShaderCache::getInstance()->fetchCompiledGlProgram(getRGBFrameShaderCode());
	if (m_rgbFrameShader == nullptr)
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile rgb frame shader";
		return false;
	}

	m_rgbToBgrFrameShader = GlShaderCache::getInstance()->fetchCompiledGlProgram(getRGBtoBGRVideoFrameShaderCode());
	if (m_rgbToBgrFrameShader == nullptr)
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile rgb-to-gbr frame shader";
		return false;
	}

	createVertexBuffers();

	// Load the last use compositor configuration
	m_config->load();

	if (m_config->presetName.empty())
	{
		// If no preset name is set, try the default one
		selectPreset(DEFAULT_COMPOSITOR_CONFIG_NAME);
	}
	else
	{
		// Force recreate the compositor layers for the current config
		// Save config back out if we had to add any missing mappings
		selectPreset(m_config->presetName, true);
	}

	if (!m_currentPresetConfig)
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to select initial compositor preset";
		return false;
	}

	return true;
}

void GlFrameCompositor::shutdown()
{
	stop();
	freeVertexBuffers();
	clearAllCompositorConfigurations();

	// Clean up any allocated clientSources
	for (auto iter = m_clientSources.getMap().begin(); iter != m_clientSources.getMap().end(); iter++)
	{
		GlFrameCompositor::ClientSource* clientSource = iter->second;

		clientSource->colorTexture = nullptr;
		clientSource->depthTexture = nullptr;

		delete clientSource;
	}
	m_clientSources.clear();
}

std::filesystem::path GlFrameCompositor::getCompositorPresetPath() const
{
	std::filesystem::path compositorPresetDir = PathUtils::getResourceDirectory();
	compositorPresetDir /= "config";
	compositorPresetDir /= "compositor";

	return compositorPresetDir;
}

void GlFrameCompositor::reloadAllCompositorPresets()
{
	const std::filesystem::path compositorPresetDir = getCompositorPresetPath();

	clearAllCompositorConfigurations();

	const std::vector<std::string> configFileNames = 
		PathUtils::listFilenamesInDirectory(compositorPresetDir, ".json");
	for (const auto& configFileName : configFileNames)
	{
		const std::filesystem::path configFilePath = compositorPresetDir / configFileName;

		CompositorPresetPtr compositorPreset = std::make_shared<CompositorPreset>();
		if (compositorPreset->load(configFilePath))
		{
			m_compositorPresets.setValue(compositorPreset->name, compositorPreset);
		}
		else
		{
			MIKAN_LOG_ERROR("GlFrameCompositor::reloadAllCompositorConfigurations") << "Failed to parse JSON: " << configFilePath;
		}
	}
}

void GlFrameCompositor::clearAllCompositorConfigurations()
{
	m_compositorPresets.clear();
}

bool GlFrameCompositor::addNewPreset()
{
	int nextId= m_config->nextPresetId;
	m_config->nextPresetId++;

	char szPresetName[64];
	StringUtils::formatString(szPresetName, sizeof(szPresetName), "CompositorPreset%d", nextId);

	const std::filesystem::path compositorPresetDir = getCompositorPresetPath();
	std::filesystem::path compositorPresetPath = compositorPresetDir;
	compositorPresetPath /= szPresetName;
	compositorPresetPath.replace_extension("json");

	CompositorPresetPtr newPreset = std::make_shared<CompositorPreset>(szPresetName);
	newPreset->name= szPresetName;

	newPreset->save(compositorPresetPath.string());	
	m_compositorPresets.setValue(szPresetName, newPreset);

	return selectPreset(szPresetName);
}

bool GlFrameCompositor::deleteCurrentPreset()
{
	CompositorPresetPtr preset = getCurrentPresetConfigMutable();
	if (preset == nullptr)
		return false;

	// Not allowed to delete built in presets
	if (preset->builtIn)
		return false;

	// Delete the preset config off disk
	const std::filesystem::path presetPath= preset->getLoadedConfigPath();
	if (!std::filesystem::remove(presetPath))
		return false;

	// Clear out the preset from memory
	m_compositorPresets.removeValue(preset->name);

	// Drop back to the default built-in preset
	return selectPreset(DEFAULT_COMPOSITOR_CONFIG_NAME);
}

bool GlFrameCompositor::setCurrentPresetName(const std::string& newPresetName)
{
	CompositorPresetPtr preset = getCurrentPresetConfigMutable();
	if (preset == nullptr)
		return false;

	// Bail if the name didn't actually change
	if (preset->name == newPresetName)
		return false;

	// for built-in presets, say we succeeded, but do nothing so that we refresh the UI back to defaults
	if (preset->builtIn)
		return true;

	// Update the current preset name
	m_compositorPresets.removeValue(preset->name);
	preset->name= newPresetName;
	m_config->presetName= newPresetName;
	m_compositorPresets.setValue(preset->name, preset);

	m_config->markDirty(ConfigPropertyChangeSet().addPropertyName(GlFrameCompositorConfig::k_presetNamePropertyId));
	saveCurrentPresetConfig();

	return true;
}

void GlFrameCompositor::saveCurrentPresetConfig()
{
	CompositorPresetPtr preset= getCurrentPresetConfigMutable();
	if (preset != nullptr)
	{
		preset->save(preset->getLoadedConfigPath());
	}
}

const std::filesystem::path& GlFrameCompositor::getCompositorGraphAssetPath() const
{
	return m_nodeGraphAssetRef->getAssetPath();
}

void GlFrameCompositor::setCompositorGraphAssetPath(
	const std::filesystem::path& assetRefPath)
{
	CompositorPresetPtr preset = getCurrentPresetConfigMutable();
	if (preset && preset->compositorGraphAssetRefConfig->assetPath != assetRefPath.string())
	{
		preset->compositorGraphAssetRefConfig->assetPath = assetRefPath.string();

		// Signal to any listeners that the asset path changed
		preset->markDirty(
			ConfigPropertyChangeSet().addPropertyName(
				CompositorPreset::k_compositorGraphAssetRefPropertyId));

		// Save the preset immediately to disk
		preset->save(preset->getLoadedConfigPath());
	}
}

std::vector<std::string> GlFrameCompositor::getPresetNames() const
{
	std::vector<std::string> configurationNames;

	for (auto it = m_compositorPresets.getMap().begin(); it != m_compositorPresets.getMap().end(); it++)
	{
		configurationNames.push_back(it->first);
	}

	return configurationNames;
}

const std::string& GlFrameCompositor::getCurrentPresetName() const
{
	return m_config->presetName;
}

bool GlFrameCompositor::selectPreset(const std::string& presetName, bool bForce)
{
	CompositorPresetPtr newPresetConfig;
	if ((presetName != m_config->presetName || bForce) && 
		m_compositorPresets.tryGetValue(presetName, newPresetConfig))
	{
		// Stop listening to config changes from the old preset
		if (m_currentPresetConfig)
		{
			m_currentPresetConfig->OnMarkedDirty -=
				MakeDelegate(this, &GlFrameCompositor::onPresetConfigMarkedDirty);
		}

		m_config->presetName= presetName;
		m_currentPresetConfig= newPresetConfig;

		// Start listening to config changes from the new preset
		m_currentPresetConfig->OnMarkedDirty +=
			MakeDelegate(this, &GlFrameCompositor::onPresetConfigMarkedDirty);

		// Load the compositor graph if the config has a valid asset path
		{
			AssetReferenceConfigPtr assetRefConfigPtr = m_currentPresetConfig->compositorGraphAssetRefConfig;
			const std::string assetRefPath = assetRefConfigPtr ? assetRefConfigPtr->assetPath : "";

			onCompositorGraphAssetRefChanged(assetRefPath);
		}

		// Notify listeners that the preset has changed and mark the config as dirty
		m_config->markDirty(ConfigPropertyChangeSet().addPropertyName(GlFrameCompositorConfig::k_presetNamePropertyId));

		// Write the compositor configuration back out (and reset the dirty flag)
		m_config->save();

		return true;
	}

	return false;
}

void GlFrameCompositor::onPresetConfigMarkedDirty(
	CommonConfigPtr configPtr,
	const ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(CompositorPreset::k_compositorGraphAssetRefPropertyId))
	{
		onCompositorGraphAssetRefChanged(m_currentPresetConfig->compositorGraphAssetRefConfig->assetPath);
	}
}

void GlFrameCompositor::onCompositorGraphAssetRefChanged(const std::string& assetRefPath)
{
	if (m_nodeGraphAssetRef->getAssetPath() != assetRefPath)
	{
		m_nodeGraphAssetRef->setAssetPath(assetRefPath);

		if (m_nodeGraphAssetRef->isValid())
		{
			m_nodeGraph =
				std::dynamic_pointer_cast<CompositorNodeGraph>(
					NodeGraphFactory::loadNodeGraph(m_ownerWindow, assetRefPath));
			if (!m_nodeGraph)
			{
				MIKAN_LOG_ERROR("GlFrameCompositor::setCompositorGraphAssetPath")
					<< "Failed to load compositor graph: " << assetRefPath;
			}
		}
		else
		{
			m_nodeGraph = nullptr;
		}
	}
}

bool GlFrameCompositor::start()
{
	if (getIsRunning())
		return true;

	if (!openVideoSource())
		return false;

	// Try to bind to the camera VR tracker (ok to fail)
	bindCameraVRTracker();

	// Initialize layers for any active Mikan client connections
	{
		MikanServer* mikanServer = MikanServer::getInstance();

		// Create layers for all connected clients with allocated render targets
		std::vector<MikanClientConnectionInfo> clientList;
		mikanServer->getConnectedClientInfoList(clientList);
		for (const MikanClientConnectionInfo& connectionInfo : clientList)
		{
			if (connectionInfo.hasAllocatedRenderTarget())
			{
				onClientRenderTargetAllocated(
					connectionInfo.clientId, 
					connectionInfo.clientInfo,
					connectionInfo.renderTargetReadAccessor);
			}
		}

		// Listen for new render target events
		mikanServer->OnClientRenderTargetAllocated += MakeDelegate(this, &GlFrameCompositor::onClientRenderTargetAllocated);
		mikanServer->OnClientRenderTargetReleased += MakeDelegate(this, &GlFrameCompositor::onClientRenderTargetReleased);
		mikanServer->OnClientRenderTargetUpdated += MakeDelegate(this, &GlFrameCompositor::onClientRenderTargetUpdated);

		m_bIsRunning= true;
		m_timeSinceLastFrameComposited= 0.f;
	}

	return true;
}

void GlFrameCompositor::stop()
{
	// Stop listening to render target events
	{
		MikanServer* mikanServer = MikanServer::getInstance();

		mikanServer->OnClientRenderTargetAllocated -= MakeDelegate(this, &GlFrameCompositor::onClientRenderTargetAllocated);
		mikanServer->OnClientRenderTargetReleased -= MakeDelegate(this, &GlFrameCompositor::onClientRenderTargetReleased);
		mikanServer->OnClientRenderTargetUpdated -= MakeDelegate(this, &GlFrameCompositor::onClientRenderTargetUpdated);
	}

	m_bIsRunning = false;
}

bool GlFrameCompositor::getVideoSourceCameraPose(glm::mat4& outCameraMat) const
{
	if (m_videoSourceView != nullptr && m_cameraTrackingPuckView != nullptr)
	{
		outCameraMat= m_videoSourceView->getCameraPose(m_cameraTrackingPuckView);
		return true;
	}
	
	return false;
}

bool GlFrameCompositor::getVideoSourceViewProjection(glm::mat4& outCameraVP) const
{
	if (m_videoSourceView != nullptr && m_cameraTrackingPuckView != nullptr)
	{
		outCameraVP = m_videoSourceView->getCameraViewProjectionMatrix(m_cameraTrackingPuckView);
		return true;
	}

	return false;
}

GlTexturePtr GlFrameCompositor::getVideoSourceTexture(eVideoTextureSource textureSource) const
{
	if (m_videoDistortionView != nullptr)
	{
		switch (textureSource)
		{
			case eVideoTextureSource::video_texture:
				return m_videoDistortionView->getVideoTexture();
			case eVideoTextureSource::distortion_texture:
				return m_videoDistortionView->getDistortionTexture();
		}
	}

	return GlTexturePtr();
}

GlTexturePtr GlFrameCompositor::getClientSourceTexture(int clientIndex, eClientTextureType clientTextureType) const
{
	for (auto it = m_clientSources.getMap().begin(); it != m_clientSources.getMap().end(); it++)
	{
		ClientSource* clientSource= it->second;

		if (clientSource->clientSourceIndex == clientIndex)
		{
			switch (clientTextureType)
			{
				case eClientTextureType::color:
					return clientSource->colorTexture;
				case eClientTextureType::depth:
					return clientSource->depthTexture;
			}
		}
	}

	return GlTexturePtr();
}

void GlFrameCompositor::update(float deltaSeconds)
{
	EASY_FUNCTION();

	if (!getIsRunning())
		return;

	if (!m_cameraTrackingPuckView)
	{
		bindCameraVRTracker();
	}

	glm::mat4 cameraXform;
	if (!getVideoSourceCameraPose(cameraXform))
	{
		cameraXform= glm::mat4(1.f);
	}

	// Keep track of how long it's been since the last frame has been composited
	// This is used to update the timer in compositorNodeGraph
	m_timeSinceLastFrameComposited+= deltaSeconds;

	// Composite the next frame if we got all the renders back from the clients
	if (m_pendingCompositeFrameIndex != 0)
	{
		// See if all client render targets have been updated
		size_t clientSourceReadyCount = 0;
		for (auto it = m_clientSources.getMap().begin(); it != m_clientSources.getMap().end(); it++)
		{
			const GlFrameCompositor::ClientSource* clientSource= it->second;

			if (!clientSource->bIsPendingRender)
			{
				clientSourceReadyCount++;
			}
		}

		// If the video frame and client sources are fresh, composite them together
		if (clientSourceReadyCount == m_clientSources.getMap().size())
		{
			// Pop the frame event from the queue now that we are compositing it
			assert(m_frameEventQueue.front().frame == m_pendingCompositeFrameIndex);
			m_frameEventQueue.pop();

			MIKAN_LOG_TRACE("GlFrameCompositor::update") << "Composite frame " << m_pendingCompositeFrameIndex;
			updateCompositeFrame();
		}
	}

	// Fetch new video frames if the video frame queue isn't full
	if (m_videoDistortionView->hasNewVideoFrame())
	{
		// If the queue is full, drop all queued frames to catch up
		if (m_frameEventQueue.size() < m_videoDistortionView->getMaxFrameQueueSize())
		{
			m_lastReadVideoFrameIndex = m_videoDistortionView->readNextVideoFrame();

			MikanVideoSourceNewFrameEvent newFrameEvent;
			memset(&newFrameEvent, 0, sizeof(MikanVideoSourceNewFrameEvent));
			newFrameEvent.frame = m_lastReadVideoFrameIndex;

			const glm::vec3 cameraUp(cameraXform[1]); // Camera up is along the y-axis
			const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
			const glm::vec3 cameraPosition(cameraXform[3]); // Camera up is along the y-axis
			newFrameEvent.cameraForward = glm_vec3_to_MikanVector3f(cameraForward);
			newFrameEvent.cameraUp = glm_vec3_to_MikanVector3f(cameraUp);
			newFrameEvent.cameraPosition = glm_vec3_to_MikanVector3f(cameraPosition);

			MIKAN_LOG_TRACE("GlFrameCompositor::update") << "Enqueue frame " << m_lastReadVideoFrameIndex;
			m_frameEventQueue.push(newFrameEvent);
			m_droppedFrameCounter= 0;
		}
		else
		{
			m_droppedFrameCounter++;
			MIKAN_LOG_WARNING("GlFrameCompositor::update") << "Frame queue overflow. Dropped " << m_droppedFrameCounter << " frames";

			if (m_droppedFrameCounter > 10)
			{
				m_droppedFrameCounter= 0;
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
		MikanVideoSourceNewFrameEvent newFrameEvent= m_frameEventQueue.front();

		// Mark all client sources as pending
		for (auto it = m_clientSources.getMap().begin(); it != m_clientSources.getMap().end(); it++)
		{
			GlFrameCompositor::ClientSource* clientSource = it->second;

			clientSource->bIsPendingRender= true;
		}

		// Track the index of the pending frame
		m_pendingCompositeFrameIndex = newFrameEvent.frame;

		// Tell all clients that we have a new frame to render
		MIKAN_LOG_TRACE("GlFrameCompositor::update") << "Send frame " << m_pendingCompositeFrameIndex;
		MikanServer::getInstance()->publishVideoSourceNewFrameEvent(newFrameEvent);
	}
}

void GlFrameCompositor::updateCompositeFrame()
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

	// Optionally bake our a BGR video frame, if requested
	if (m_bGenerateBGRVideoTexture)
	{
		EASY_BLOCK("Render BGR Frame")

		// bind to bgr framebuffer and draw composited frame, but use shader to convert from RGB to BGR
		glBindFramebuffer(GL_FRAMEBUFFER, m_bgrFramebuffer);

		m_rgbToBgrFrameShader->bindProgram();
		
		std::string uniformName;
		m_rgbToBgrFrameShader->getFirstUniformNameOfSemantic(eUniformSemantic::texture0, uniformName);
		m_rgbToBgrFrameShader->setTextureUniform(uniformName);

		GlTextureConstPtr compositedFrameTexture= getCompositedFrameTexture();
		if (compositedFrameTexture)
		{
			compositedFrameTexture->bindTexture(0);

			glBindVertexArray(m_videoQuadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);

			compositedFrameTexture->clearTexture(0);
		}

		m_rgbToBgrFrameShader->unbindProgram();

		// unbind the bgr frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Remember the index of the last frame we composited
	m_lastCompositedFrameIndex = m_pendingCompositeFrameIndex;

	// Clear the pending composite frame index
	m_pendingCompositeFrameIndex = 0;

	// Reset the time since the last frame was composited
	m_timeSinceLastFrameComposited= 0.f;

	// Tell any listeners that a new frame was composited
	if (OnNewFrameComposited)
	{
		OnNewFrameComposited();
	}
}

void GlFrameCompositor::setCompositorEvaluatorWindow(eCompositorEvaluatorWindow evalWindow)
{
	if (m_evaluatorWindow != evalWindow)
	{
		m_editorFrameBufferTexture->disposeTexture();

		if (evalWindow == eCompositorEvaluatorWindow::editorWindow)
		{
			m_editorFrameBufferTexture->createTexture();
		}

		m_evaluatorWindow= evalWindow;
	}
}

GlTexturePtr GlFrameCompositor::getEditorWritableFrameTexture() const
{
	return m_editorFrameBufferTexture;
}

GlTextureConstPtr GlFrameCompositor::getCompositedFrameTexture() const
{
	switch (m_evaluatorWindow)
	{
		case eCompositorEvaluatorWindow::mainWindow:
			{
				// TODO: Simplify this once the switch to only using the node graph
				if (m_nodeGraph)
					return m_nodeGraph->getCompositedFrameTexture();
				else
					return m_mainWindowFrameBufferTexture;
			}
			break;
		case eCompositorEvaluatorWindow::editorWindow:
			return m_editorFrameBufferTexture;
	}

	return GlTextureConstPtr();
}

void GlFrameCompositor::updateCompositeFrameNodeGraph()
{
	MainWindow* mainWindow = MainWindow::getInstance();
	NodeEvaluator evaluator = {};
	evaluator
		.setCurrentVideoSourceView(m_videoSourceView)
		.setCurrentWindow(mainWindow)
		.setDeltaSeconds(m_timeSinceLastFrameComposited);

	if (!m_nodeGraph->compositeFrame(evaluator))
	{
		for (const NodeEvaluationError& error : evaluator.getErrors())
		{
			MIKAN_LOG_ERROR("GlFrameCompositor::updateCompositeFrame")
				<< "Compositor graph eval error: " << error.errorMessage;
		}
	}
}

void GlFrameCompositor::render() const
{
	if (!getIsRunning())
		return;

	GlTextureConstPtr compositedFrameTexture = getCompositedFrameTexture();
	if (compositedFrameTexture)
	{
		GlScopedState scopedState= MainWindow::getInstance()->getGlStateStack().createScopedState();
		scopedState.getStackState().disableFlag(eGlStateFlagType::depthTest);

		// Draw the composited video frame
		m_rgbFrameShader->bindProgram();
		compositedFrameTexture->bindTexture();
		glBindVertexArray(m_layerQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		compositedFrameTexture->clearTexture();
		m_rgbFrameShader->unbindProgram();
	}
}

bool GlFrameCompositor::openVideoSource()
{
	ProfileConfigConstPtr profileConfig = App::getInstance()->getProfileConfig();

	m_videoSourceView= VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();
	bool bSuccess= m_videoSourceView != nullptr;

	// Start streaming the video
	if (bSuccess && !m_videoSourceView->startVideoStream())
		bSuccess= false;

	// Create a frame buffer and texture to do the compositing work in
	uint16_t frameWidth= 0;
	uint16_t frameHeight= 0;
	if (bSuccess)
	{
		frameWidth = (uint16_t)m_videoSourceView->getFrameWidth();
		frameHeight = (uint16_t)m_videoSourceView->getFrameHeight();
		bSuccess= createLayerCompositingFrameBuffer(frameWidth, frameHeight);
	}

	// Create a frame buffer and texture to convert composited texture to BGR video frame
	if (bSuccess)
	{
		bSuccess = createBGRVideoFrameBuffer(frameWidth, frameHeight);
	}

	if (bSuccess)
	{
		// Create a distortion view to read the incoming video frames into a texture
		m_videoDistortionView =
			new VideoFrameDistortionView(
				m_videoSourceView,
				VIDEO_FRAME_HAS_BGR_UNDISTORT_FLAG | VIDEO_FRAME_HAS_GL_TEXTURE_FLAG,
				profileConfig->videoFrameQueueSize);

		// Just pass the raw video frame straight to the bgr texture
		// The frame compositor will do the undistortion work in a shader
		m_videoDistortionView->setVideoDisplayMode(eVideoDisplayMode::mode_bgr);
		m_videoDistortionView->setColorUndistortDisabled(true);
	}
	else
	{
		// Clean up the partially opened compositor
		closeVideoSource();
	}

	return bSuccess;
}

void GlFrameCompositor::closeVideoSource()
{
	freeLayerFrameBuffer();
	freeBGRVideoFrameBuffer();

	if (m_videoDistortionView != nullptr)
	{
		delete m_videoDistortionView;
		m_videoDistortionView= nullptr;
	}

	if (m_videoSourceView)
	{
		m_videoSourceView->stopVideoStream();
		m_videoSourceView= nullptr;
	}
}

bool GlFrameCompositor::bindCameraVRTracker()
{
	ProfileConfigConstPtr profileConfig = App::getInstance()->getProfileConfig();

	m_cameraTrackingPuckView= 
		VRDeviceManager::getInstance()->getVRDeviceViewByPath(profileConfig->cameraVRDevicePath);

	return m_cameraTrackingPuckView != nullptr;
}

bool GlFrameCompositor::addClientSource(
	const std::string& clientId, 
	const MikanClientInfo& clientInfo,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	if (m_clientSources.hasValue(clientId))
		return false;

	GlFrameCompositor::ClientSource* clientSource = new GlFrameCompositor::ClientSource;
	memset(clientSource, 0, sizeof(GlFrameCompositor::ClientSource));

	const MikanRenderTargetDescriptor& desc= readAccessor->getRenderTargetDescriptor();
	clientSource->clientSourceIndex= m_clientSources.getNumEntries();
	clientSource->clientId = clientId;
	clientSource->clientInfo = clientInfo;
	clientSource->desc = desc;
	clientSource->frameIndex = 0;

	switch (desc.color_buffer_type)
	{
	case MikanColorBuffer_RGB24:
		clientSource->colorTexture = std::make_shared<GlTexture>();
		clientSource->colorTexture->setTextureFormat(GL_RGB);
		clientSource->colorTexture->setBufferFormat(GL_RGB);
		break;
	case MikanColorBuffer_RGBA32:
		clientSource->colorTexture = std::make_shared<GlTexture>();
		clientSource->colorTexture->setTextureFormat(GL_RGBA);
		clientSource->colorTexture->setBufferFormat(GL_RGBA);
		break;
	case MikanColorBuffer_BGRA32:
		clientSource->colorTexture = std::make_shared<GlTexture>();
		clientSource->colorTexture->setTextureFormat(GL_BGRA);
		clientSource->colorTexture->setBufferFormat(GL_BGRA);
		break;
	}

	if (clientSource->colorTexture != nullptr)
	{
		clientSource->colorTexture->setSize(desc.width, desc.height);
		clientSource->colorTexture->setGenerateMipMap(false);
		clientSource->colorTexture->setPixelBufferObjectMode(
			desc.graphicsAPI == MikanClientGraphicsApi_UNKNOWN 
			? GlTexture::PixelBufferObjectMode::DoublePBOWrite
			: GlTexture::PixelBufferObjectMode::NoPBO);
		clientSource->colorTexture->createTexture();

		readAccessor->setColorTexture(clientSource->colorTexture);
	}

	switch (desc.depth_buffer_type)
	{
	case MikanDepthBuffer_DEPTH16:
		clientSource->depthTexture = std::make_shared<GlTexture>();
		clientSource->depthTexture->setTextureFormat(GL_DEPTH_COMPONENT16);
		clientSource->depthTexture->setBufferFormat(GL_DEPTH_COMPONENT);
		break;
	case MikanDepthBuffer_DEPTH32:
		clientSource->depthTexture = std::make_shared<GlTexture>();
		clientSource->depthTexture->setTextureFormat(GL_DEPTH_COMPONENT32F);
		clientSource->depthTexture->setBufferFormat(GL_DEPTH_COMPONENT);
		break;
	}

	if (clientSource->depthTexture != nullptr)
	{
		clientSource->depthTexture->setSize(desc.width, desc.height);
		clientSource->depthTexture->setPixelBufferObjectMode(
			desc.graphicsAPI == MikanClientGraphicsApi_UNKNOWN
			? GlTexture::PixelBufferObjectMode::DoublePBOWrite
			: GlTexture::PixelBufferObjectMode::NoPBO);
		clientSource->depthTexture->createTexture();

		readAccessor->setDepthTexture(clientSource->depthTexture);
	}

	// Add the client source to the data source table
	m_clientSources.setValue(clientId, clientSource);

	return true;
}

bool GlFrameCompositor::removeClientSource(
	const std::string& clientId,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	GlFrameCompositor::ClientSource* clientSource = m_clientSources.getValueOrDefault(clientId, nullptr);
	if (clientSource == nullptr)
		return false;

	clientSource->colorTexture = nullptr;
	readAccessor->setColorTexture(nullptr);

	clientSource->depthTexture= nullptr;
	readAccessor->setDepthTexture(nullptr);

	// Remove the client source entries from the data source tables
	m_clientSources.removeValue(clientId);

	// NOTE: There may still be layers that refer to this now invalid clientID but that is ok.
	// We want to allow the existing layer config to work again if the client source reconnects.
	// The existing layer will just stop drawing while the associated data sources are invalid.

	delete clientSource;

	return true;
}

bool GlFrameCompositor::createLayerCompositingFrameBuffer(uint16_t width, uint16_t height)
{
	bool bSuccess = true;

	glGenFramebuffers(1, &m_layerFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_layerFramebuffer);

	// create a color attachment texture
	m_mainWindowFrameBufferTexture = std::make_shared<GlTexture>();
	m_mainWindowFrameBufferTexture->setSize(width, height);
	m_mainWindowFrameBufferTexture->setTextureFormat(GL_RGBA);
	m_mainWindowFrameBufferTexture->setBufferFormat(GL_RGBA);
	m_mainWindowFrameBufferTexture->setGenerateMipMap(false);
	m_mainWindowFrameBufferTexture->createTexture();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mainWindowFrameBufferTexture->getGlTextureId(), 0);

	// Also create a for the editor to render to when the editor is active
	m_editorFrameBufferTexture = std::make_shared<GlTexture>();
	m_editorFrameBufferTexture->setSize(width, height);
	m_editorFrameBufferTexture->setTextureFormat(GL_RGBA);
	m_editorFrameBufferTexture->setBufferFormat(GL_RGBA);
	m_editorFrameBufferTexture->setGenerateMipMap(false);
	// Don't create texture until we need it

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &m_layerRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_layerRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_layerRBO); // now actually attach it

	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		MIKAN_LOG_ERROR("createFrameBuffer") << "Framebuffer is not complete!";
		bSuccess = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return bSuccess;
}

void GlFrameCompositor::freeLayerFrameBuffer()
{
	if (m_layerRBO != 0)
	{
		glDeleteRenderbuffers(1, &m_layerRBO);
		m_layerRBO = 0;
	}

	m_mainWindowFrameBufferTexture = nullptr;

	if (m_layerFramebuffer != 0)
	{
		glDeleteFramebuffers(1, &m_layerFramebuffer);
		m_layerFramebuffer = 0;
	}
}

bool GlFrameCompositor::createBGRVideoFrameBuffer(uint16_t width, uint16_t height)
{
	bool bSuccess = true;

	glGenFramebuffers(1, &m_bgrFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_bgrFramebuffer);

	// create a color attachment texture with a double buffered pixel-buffer-object for reading
	m_bgrVideoFrame = std::make_shared<GlTexture>();
	m_bgrVideoFrame->setSize(width, height);
	m_bgrVideoFrame->setTextureFormat(GL_RGB);
	m_bgrVideoFrame->setBufferFormat(GL_RGB);
	m_bgrVideoFrame->setGenerateMipMap(false);
	m_bgrVideoFrame->setPixelBufferObjectMode(GlTexture::PixelBufferObjectMode::DoublePBORead);
	m_bgrVideoFrame->createTexture();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bgrVideoFrame->getGlTextureId(), 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	glGenRenderbuffers(1, &m_bgrRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_bgrRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_bgrRBO); // now actually attach it

	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		MIKAN_LOG_ERROR("createBGRVideoFrameBuffer") << "Framebuffer is not complete!";
		bSuccess = false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return bSuccess;
}

void GlFrameCompositor::freeBGRVideoFrameBuffer()
{
	if (m_bgrRBO != 0)
	{
		glDeleteRenderbuffers(1, &m_bgrRBO);
		m_bgrRBO = 0;
	}

	m_bgrVideoFrame = nullptr;

	if (m_bgrFramebuffer != 0)
	{
		glDeleteFramebuffers(1, &m_bgrFramebuffer);
		m_bgrFramebuffer = 0;
	}
}

void GlFrameCompositor::createVertexBuffers()
{
	// vertex attributes for quad that fills the entire screen in Normalized Device Coordinates.
	const float layerQuadVertices[] = { 
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	const float videoQuadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 0.0f,
		-1.0f, -1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 1.0f,

		-1.0f,  1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 0.0f
	};

	// layer quad VAO/VBO
	glGenVertexArrays(1, &m_layerQuadVAO);
	glGenBuffers(1, &m_layerQuadVBO);
	glBindVertexArray(m_layerQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_layerQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(layerQuadVertices), &layerQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// video quad VAO/VBO
	glGenVertexArrays(1, &m_videoQuadVAO);
	glGenBuffers(1, &m_videoQuadVBO);
	glBindVertexArray(m_videoQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_videoQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(videoQuadVertices), &videoQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void GlFrameCompositor::freeVertexBuffers()
{
	if (m_layerQuadVAO != 0)
	{
		glDeleteVertexArrays(1, &m_layerQuadVAO);
		m_layerQuadVAO = 0;
	}
	if (m_layerQuadVBO != 0)
	{
		glDeleteBuffers(1, &m_layerQuadVBO);
		m_layerQuadVBO = 0;
	}

	if (m_videoQuadVAO != 0)
	{
		glDeleteVertexArrays(1, &m_videoQuadVAO);
		m_videoQuadVAO = 0;
	}
	if (m_videoQuadVBO != 0)
	{
		glDeleteBuffers(1, &m_videoQuadVBO);
		m_videoQuadVBO = 0;
	}
}

// MikanServer Events
void GlFrameCompositor::onClientRenderTargetAllocated(
	const std::string& clientId, 
	const MikanClientInfo& clientInfo,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	addClientSource(clientId, clientInfo, readAccessor);
}

void GlFrameCompositor::onClientRenderTargetReleased(
	const std::string& clientId,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	removeClientSource(clientId, readAccessor);
}

void GlFrameCompositor::onClientRenderTargetUpdated(
	const std::string& clientId, 
	uint64_t frameIndex)
{
	EASY_FUNCTION();

	MIKAN_LOG_TRACE("GlFrameCompositor::onClientRenderTargetUpdated") << "Recv frame " << frameIndex;

	GlFrameCompositor::ClientSource* clientSource;	
	if (m_clientSources.tryGetValue(clientId, clientSource))
	{
		// Update the frame index
		clientSource->frameIndex = frameIndex;

		// Mark that the client source is no longer pending the render
		//assert(clientSource->bIsPendingRender);
		clientSource->bIsPendingRender = false;
	}
}

const GlProgramCode* GlFrameCompositor::getRGBFrameShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"Internal RGB Frame Shader Code",
		// vertex shader
		R""""(
			#version 330 core
			layout (location = 0) in vec2 aPos;
			layout (location = 1) in vec2 aTexCoords;

			out vec2 TexCoords;

			void main()
			{
				TexCoords = aTexCoords;
				gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
			}  
			)"""",
		//fragment shader
		R""""(
			#version 330 core
			out vec4 FragColor;

			in vec2 TexCoords;

			uniform sampler2D rgbTexture;

			void main()
			{
				vec3 col = texture(rgbTexture, TexCoords).rgb;
				FragColor = vec4(col, 1.0);
			} 
			)"""")
		.addUniform("rgbTexture", eUniformSemantic::texture0);

	return &x_shaderCode;
}

const GlProgramCode* GlFrameCompositor::getRGBtoBGRVideoFrameShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"Internal BGR Frame Shader Code",
		// vertex shader
		R""""(
			#version 330 core
			layout (location = 0) in vec2 aPos;
			layout (location = 1) in vec2 aTexCoords;

			out vec2 TexCoords;

			void main()
			{
				TexCoords = aTexCoords;
				gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
			}  
			)"""",
		//fragment shader
		R""""(
			#version 330 core
			out vec4 FragColor;

			in vec2 TexCoords;

			uniform sampler2D rgbTexture;

			void main()
			{
				vec3 col = texture(rgbTexture, TexCoords).bgr;
				FragColor = vec4(col, 1.0);
			} 
			)"""")
		.addUniform("rgbTexture", eUniformSemantic::texture0);

	return &x_shaderCode;
}