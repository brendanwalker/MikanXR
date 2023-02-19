#include "App.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlFrameCompositor.h"
#include "GlMaterial.h"
#include "GlTexture.h"
#include "GlTextRenderer.h"
#include "GlProgramConfig.h"
#include "InterprocessRenderTargetReader.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "PathUtils.h"
#include "ProfileConfig.h"
#include "Renderer.h"
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

#include <algorithm>

#include <easy/profiler.h>

#define DEFAULT_COMPOSITOR_CONFIG_NAME	"Alpha Channel"
#define STENCIL_MVP_UNIFORM_NAME		"mvpMatrix"
#define MAX_CLIENT_SOURCES				8

// -- GlFrameCompositor ------
GlFrameCompositor* GlFrameCompositor::m_instance= nullptr;

GlFrameCompositor::GlFrameCompositor()
{
	m_instance= this;
}

GlFrameCompositor::~GlFrameCompositor()
{
	m_instance = nullptr;
}

bool GlFrameCompositor::startup()
{
	EASY_FUNCTION();

	reloadAllCompositorShaders();
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

	m_stencilShader = GlShaderCache::getInstance()->fetchCompiledGlProgram(getStencilShaderCode());
	if (m_stencilShader == nullptr)
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile stencil shader";
		return false;
	}

	createVertexBuffers();

	// Load the last use compositor configuration
	m_config.load();

	if (m_config.presetName.empty())
	{
		// If no preset name is set, try the default one
		applyLayerPreset(DEFAULT_COMPOSITOR_CONFIG_NAME);
	}
	else
	{
		// Recreate the compositor layers for the current config
		rebuildLayersFromConfig();
	}

	// Create data source defaults
	for (int clientSourceIndex= 0; clientSourceIndex < MAX_CLIENT_SOURCES; ++clientSourceIndex)
	{
		const std::string colorTextureSourceName = makeClientRendererTextureName(clientSourceIndex);
		m_colorTextureSources.setValue(colorTextureSourceName, nullptr);

		const std::string colorKeySourceName = makeClientColorKeyName(clientSourceIndex);
		m_float3Sources.setValue(colorKeySourceName, Colors::Black);
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

		if (clientSource->colorTexture != nullptr)
		{
			clientSource->colorTexture->disposeTexture();
			delete clientSource->colorTexture;
		}

		if (clientSource->depthTexture != nullptr)
		{
			clientSource->depthTexture->disposeTexture();
			delete clientSource->depthTexture;
		}

		delete clientSource;
	}
	m_clientSources.clear();

	// Clean up any allocated materials
	for (auto iter = m_materialSources.getMap().begin(); iter != m_materialSources.getMap().end(); iter++)
	{
		delete iter->second;
	}
	m_materialSources.clear();
}

void GlFrameCompositor::reloadAllCompositorPresets()
{
	std::filesystem::path compositorShaderDir = PathUtils::getResourceDirectory();
	compositorShaderDir /= "config";
	compositorShaderDir /= "compositor";

	clearAllCompositorConfigurations();

	std::vector<std::string> configFileNames = PathUtils::listFiles(compositorShaderDir.string(), "json");
	for (const auto& configFileName : configFileNames)
	{
		std::filesystem::path configFilePath = compositorShaderDir;
		configFilePath /= configFileName;

		CompositorPreset* compositorPreset = new CompositorPreset;
		if (compositorPreset->load(configFilePath.string()))
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
	for (auto it = m_compositorPresets.getMap().begin(); it != m_compositorPresets.getMap().end(); it++)
	{
		delete it->second;
	}
	m_compositorPresets.clear();
}

const CompositorPreset* GlFrameCompositor::getCurrentPresetConfig() const
{
	CompositorPreset* preset = nullptr;
	if (m_compositorPresets.tryGetValue(m_config.presetName, preset))
	{
		return preset;
	}

	return nullptr;
}

CompositorPreset* GlFrameCompositor::getCurrentPresetConfigMutable() const
{
	return const_cast<CompositorPreset*>(getCurrentPresetConfig());
}

const CompositorLayerConfig* GlFrameCompositor::getCurrentPresetLayerConfig(int layerIndex) const
{
	const CompositorPreset* preset = getCurrentPresetConfig();
	if (preset != nullptr)
	{
		if (layerIndex >= 0 && layerIndex < (int)preset->layers.size())
		{
			return &preset->layers[layerIndex];
		}
	}

	return nullptr;
}

CompositorLayerConfig* GlFrameCompositor::getCurrentPresetLayerConfigMutable(int layerIndex)
{
	return const_cast<CompositorLayerConfig*>(getCurrentPresetLayerConfig(layerIndex));
}

void GlFrameCompositor::saveCurrentPresetConfig()
{
	CompositorPreset* preset= getCurrentPresetConfigMutable();
	if (preset != nullptr)
	{
		preset->save(preset->getLoadedConfigPath());
	}
}

void GlFrameCompositor::rebuildLayersFromConfig()
{
	CompositorPreset* preset = nullptr;
	if (m_compositorPresets.tryGetValue(m_config.presetName, preset))
	{
		m_layers.clear();
		for (int layerIndex = 0; layerIndex < (int)preset->layers.size(); ++layerIndex)
		{
			const CompositorLayerConfig& layerConfig = preset->layers[layerIndex];
			GlMaterial* material = m_materialSources.getValueOrDefault(layerConfig.shaderConfig.materialName, nullptr);

			const Layer layer = {layerIndex, material, -1};
			m_layers.push_back(layer);
		}
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
	return m_config.presetName;
}

bool GlFrameCompositor::applyLayerPreset(const std::string& presetName)
{
	if (presetName == m_config.presetName)
		return false;

	if (m_compositorPresets.hasValue(presetName))
	{
		m_config.presetName= presetName;
		m_config.save();

		// Recreate the compositor layers for the current config
		rebuildLayersFromConfig();
	}

	return true;
}

void GlFrameCompositor::reloadAllCompositorShaders()
{
	std::filesystem::path compositorShaderDir= PathUtils::getResourceDirectory();
	compositorShaderDir/= "shaders";
	compositorShaderDir/= "compositor";

	std::vector<std::string> shaderFolderNames= PathUtils::listDirectories(compositorShaderDir.string());
	for (const auto& shaderFolderName : shaderFolderNames)
	{
		std::filesystem::path shaderFolderPath = compositorShaderDir;
		shaderFolderPath/= shaderFolderName;

		std::vector<std::string> shaderFileNames= PathUtils::listFiles(shaderFolderPath.string(), "json");
		for (const auto& shaderFileName : shaderFileNames)
		{
			std::filesystem::path shaderFilePath = shaderFolderPath;
			shaderFilePath/= shaderFileName;

			GlProgramConfig programConfig;
			if (programConfig.load(shaderFilePath.string()))
			{
				GlProgramCode programCode;
				if (programConfig.loadGlProgramCode(&programCode))
				{
					GlProgram* program= GlShaderCache::getInstance()->fetchCompiledGlProgram(&programCode);
					GlMaterial* material= m_materialSources.getValueOrDefault(programConfig.materialName, nullptr);

					if (material != nullptr)
					{
						material->setProgram(program);
					}
					else
					{
						material = new GlMaterial(programConfig.materialName, program);
						m_materialSources.setValue(programConfig.materialName, material);
					}
				}
				else
				{
					MIKAN_LOG_ERROR("GlFrameCompositor::reloadAllCompositorShaders") << "Failed to load program code: " << shaderFilePath.string();
				}
			}
			else
			{
				MIKAN_LOG_ERROR("GlFrameCompositor::reloadAllCompositorShaders") << "Failed to parse JSON: " << shaderFilePath.string();
			}
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

void GlFrameCompositor::update()
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

void GlFrameCompositor::setFloatMapping(
	const int layerIndex,
	const std::string& uniformName,
	const std::string& dataSourceName)
{
	CompositorLayerConfig* layerConfig = getCurrentPresetLayerConfigMutable(layerIndex);
	if (layerConfig == nullptr)
		return;

	auto& sourceMap = layerConfig->shaderConfig.floatSourceMap;
	if (sourceMap.find(uniformName) != sourceMap.end() &&
		m_floatSources.hasValue(dataSourceName) &&
		sourceMap[uniformName] != dataSourceName)
	{
		sourceMap[uniformName] = dataSourceName;
		saveCurrentPresetConfig();
	}
}

void GlFrameCompositor::setFloat2Mapping(
	const int layerIndex,
	const std::string& uniformName,
	const std::string& dataSourceName)
{
	CompositorLayerConfig* layerConfig = getCurrentPresetLayerConfigMutable(layerIndex);
	if (layerConfig == nullptr)
		return;

	auto& sourceMap = layerConfig->shaderConfig.float2SourceMap;
	if (sourceMap.find(uniformName) != sourceMap.end() &&
		m_float2Sources.hasValue(dataSourceName) &&
		sourceMap[uniformName] != dataSourceName)
	{
		sourceMap[uniformName] = dataSourceName;
		saveCurrentPresetConfig();
	}
}

void GlFrameCompositor::setFloat3Mapping(
	const int layerIndex,
	const std::string& uniformName,
	const std::string& dataSourceName)
{
	CompositorLayerConfig* layerConfig = getCurrentPresetLayerConfigMutable(layerIndex);
	if (layerConfig == nullptr)
		return;

	auto& sourceMap = layerConfig->shaderConfig.float3SourceMap;
	if (sourceMap.find(uniformName) != sourceMap.end() &&
		m_float3Sources.hasValue(dataSourceName) &&
		sourceMap[uniformName] != dataSourceName)
	{
		sourceMap[uniformName] = dataSourceName;
		saveCurrentPresetConfig();
	}
}

void GlFrameCompositor::setFloat4Mapping(
	const int layerIndex,
	const std::string& uniformName,
	const std::string& dataSourceName)
{
	CompositorLayerConfig* layerConfig = getCurrentPresetLayerConfigMutable(layerIndex);
	if (layerConfig == nullptr)
		return;

	auto& sourceMap = layerConfig->shaderConfig.float4SourceMap;
	if (sourceMap.find(uniformName) != sourceMap.end() &&
		m_float4Sources.hasValue(dataSourceName) &&
		sourceMap[uniformName] != dataSourceName)
	{
		sourceMap[uniformName] = dataSourceName;
		saveCurrentPresetConfig();
	}
}

void GlFrameCompositor::setMat4Mapping(
	const int layerIndex,
	const std::string& uniformName,
	const std::string& dataSourceName)
{
	CompositorLayerConfig* layerConfig = getCurrentPresetLayerConfigMutable(layerIndex);
	if (layerConfig == nullptr)
		return;

	auto& sourceMap = layerConfig->shaderConfig.mat4SourceMap;
	if (sourceMap.find(uniformName) != sourceMap.end() &&
		m_mat4Sources.hasValue(dataSourceName) &&
		sourceMap[uniformName] != dataSourceName)
	{
		sourceMap[uniformName] = dataSourceName;
		saveCurrentPresetConfig();
	}
}

void GlFrameCompositor::setColorTextureMapping(
	const int layerIndex,
	const std::string& uniformName, 
	const std::string& dataSourceName)
{
	CompositorLayerConfig* layerConfig= getCurrentPresetLayerConfigMutable(layerIndex);
	if (layerConfig == nullptr)
		return;

	auto& sourceMap= layerConfig->shaderConfig.colorTextureSourceMap;
	if (sourceMap.find(uniformName) != sourceMap.end() &&
		m_colorTextureSources.hasValue(dataSourceName) && 
		sourceMap[uniformName] != dataSourceName)
	{
		sourceMap[uniformName]= dataSourceName;
		saveCurrentPresetConfig();
	}
}

void GlFrameCompositor::updateCompositeFrame()
{
	EASY_FUNCTION();

	assert(m_pendingCompositeFrameIndex != 0);

	// Cache the last viewport dimensions
	GLint last_viewport[4];
	glGetIntegerv(GL_VIEWPORT, last_viewport);

	// Change the viewport to match the frame buffer texture
	glViewport(0, 0, m_compositedFrame->getTextureWidth(), m_compositedFrame->getTextureHeight());

	// bind to framebuffer and draw scene as we normally would to color texture 
	glBindFramebuffer(GL_FRAMEBUFFER, m_layerFramebuffer);

	// Turn off depth testing for compositing
	Renderer* renderer= Renderer::getInstance();
	GlScopedState updateCompositeGlStateScope = renderer->getGlStateStack()->createScopedState();
	updateCompositeGlStateScope.getStackState().disableFlag(eGlStateFlagType::depthTest);

	// make sure we clear the framebuffer's content
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Read the next queued frame into a texture to composite
	if (m_videoDistortionView->processVideoFrame(m_pendingCompositeFrameIndex))
	{
		m_colorTextureSources.setValue("videoTexture", m_videoDistortionView->getVideoTexture());
		m_colorTextureSources.setValue("distortionTexture", m_videoDistortionView->getDistortionTexture());
	}
	else
	{
		m_colorTextureSources.setValue("videoTexture", nullptr);
		m_colorTextureSources.setValue("distortionTexture", nullptr);
	}

	for (GlFrameCompositor::Layer& layer : m_layers)
	{
		const CompositorLayerConfig* layerConfig= getCurrentPresetLayerConfig(layer.layerIndex);
		if (layerConfig == nullptr)
			continue;

		EASY_BLOCK(layerConfig->shaderConfig.materialName);

		GlScopedState layerGlStateScope = renderer->getGlStateStack()->createScopedState();

		// Attempt to apply data sources to the layers material parameters
		bool bValidMaterialDataSources= true;
		bValidMaterialDataSources&= applyLayerMaterialFloatValues(*layerConfig, layer);
		bValidMaterialDataSources&= applyLayerMaterialFloat2Values(*layerConfig, layer);
		bValidMaterialDataSources&= applyLayerMaterialFloat3Values(*layerConfig, layer);
		bValidMaterialDataSources&= applyLayerMaterialFloat4Values(*layerConfig, layer);
		bValidMaterialDataSources&= applyLayerMaterialMat4Values(*layerConfig, layer);
		bValidMaterialDataSources&= applyLayerMaterialTextures(*layerConfig, layer);
		if (!bValidMaterialDataSources)
			continue;

		// Set the blend mode
		switch (layerConfig->blendMode)
		{
			case eCompositorBlendMode::blendOff:
				{
					layerGlStateScope.getStackState().disableFlag(eGlStateFlagType::blend);
				}
				break;
			case eCompositorBlendMode::blendOn:
				{
					// https://www.andersriggelsen.dk/glblendfunc.php
					// (sR*sA) + (dR*(1-sA)) = rR
					// (sG*sA) + (dG*(1-sA)) = rG
					// (sB*sA) + (dB*(1-sA)) = rB
					// (sA*sA) + (dA*(1-sA)) = rA
					layerGlStateScope.getStackState().enableFlag(eGlStateFlagType::blend);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glBlendEquation(GL_FUNC_ADD);
				}
				break;
		}

		{
			GlScopedState glStateScope = Renderer::getInstance()->getGlStateStack()->createScopedState();
			GlState& glState= glStateScope.getStackState();

			// Apply stencil shapes, if any, to the layer
			updateQuadStencils(layerConfig->quadStencilConfig, &glState);
			updateBoxStencils(layerConfig->boxStencilConfig, &glState);
			updateModelStencils(layerConfig->modelStencilConfig, &glState);

			// Bind the layer shader program and uniform parameters.
			// This will fail unless all of the shader uniform parameters are bound.
			if (layer.layerMaterial != nullptr)
			{
				GlScopedMaterialBinding materialBinding = layer.layerMaterial->bindMaterial();

				if (materialBinding)
				{
					glBindVertexArray(layerConfig->verticalFlip ? m_videoQuadVAO : m_layerQuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glBindVertexArray(0);
				}
			}
		}
	}

	// Unbind the layer frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

		m_compositedFrame->bindTexture(0);

		glBindVertexArray(m_videoQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		m_compositedFrame->clearTexture(0);

		m_rgbToBgrFrameShader->unbindProgram();

		// unbind the bghr frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Restore the viewport
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);

	// Remember the index of the last frame we composited
	m_lastCompositedFrameIndex = m_pendingCompositeFrameIndex;

	// Clear the pending composite frame index
	m_pendingCompositeFrameIndex = 0;

	// Tell any listeners that a new frame was composited
	if (OnNewFrameComposited)
	{
		OnNewFrameComposited();
	}
}

bool GlFrameCompositor::applyLayerMaterialFloatValues(
	const CompositorLayerConfig& layerConfig, 
	GlFrameCompositor::Layer& layer)
{
	bool bSuccess = true;

	GlMaterial* material = layer.layerMaterial;
	for (auto it = layerConfig.shaderConfig.floatSourceMap.begin();
		 it != layerConfig.shaderConfig.floatSourceMap.end();
		 it++)
	{
		const std::string& uniformName = it->first;
		const std::string& dataSourceName = it->second;

		float floatValue;
		if (m_floatSources.tryGetValue(dataSourceName, floatValue))
		{
			bSuccess = material->setFloatByUniformName(uniformName, floatValue);
		}
		else
		{
			bSuccess = false;
		}
	}

	return bSuccess;
}

bool GlFrameCompositor::applyLayerMaterialFloat2Values(
	const CompositorLayerConfig& layerConfig, 
	GlFrameCompositor::Layer& layer)
{
	bool bSuccess = true;

	GlMaterial* material = layer.layerMaterial;
	for (auto it = layerConfig.shaderConfig.float2SourceMap.begin();
		 it != layerConfig.shaderConfig.float2SourceMap.end();
		 it++)
	{
		const std::string& uniformName = it->first;
		const std::string& dataSourceName = it->second;

		glm::vec2 float2Value;
		if (m_float2Sources.tryGetValue(dataSourceName, float2Value))
		{
			bSuccess = material->setVec2ByUniformName(uniformName, float2Value);
		}
		else
		{
			bSuccess = false;
		}
	}

	return bSuccess;
}

bool GlFrameCompositor::applyLayerMaterialFloat3Values(
	const CompositorLayerConfig& layerConfig, 
	GlFrameCompositor::Layer& layer)
{
	bool bSuccess = true;

	GlMaterial* material = layer.layerMaterial;
	for (auto it = layerConfig.shaderConfig.float3SourceMap.begin();
		 it != layerConfig.shaderConfig.float3SourceMap.end();
		 it++)
	{
		const std::string& uniformName = it->first;
		const std::string& dataSourceName = it->second;

		glm::vec3 float3Value;
		if (m_float3Sources.tryGetValue(dataSourceName, float3Value))
		{
			bSuccess = material->setVec3ByUniformName(uniformName, float3Value);
		}
		else
		{
			bSuccess = false;
		}
	}

	return bSuccess;
}

bool GlFrameCompositor::applyLayerMaterialFloat4Values(
	const CompositorLayerConfig& layerConfig, 
	GlFrameCompositor::Layer& layer)
{
	bool bSuccess = true;

	GlMaterial* material = layer.layerMaterial;
	for (auto it = layerConfig.shaderConfig.float4SourceMap.begin();
		 it != layerConfig.shaderConfig.float4SourceMap.end();
		 it++)
	{
		const std::string& uniformName = it->first;
		const std::string& dataSourceName = it->second;

		glm::vec4 float4Value;
		if (m_float4Sources.tryGetValue(dataSourceName, float4Value))
		{
			bSuccess = material->setVec4ByUniformName(uniformName, float4Value);
		}
		else
		{
			bSuccess = false;
		}
	}

	return bSuccess;
}

bool GlFrameCompositor::applyLayerMaterialMat4Values(
	const CompositorLayerConfig& layerConfig, 
	GlFrameCompositor::Layer& layer)
{
	bool bSuccess = true;

	GlMaterial* material = layer.layerMaterial;
	for (auto it = layerConfig.shaderConfig.mat4SourceMap.begin();
		 it != layerConfig.shaderConfig.mat4SourceMap.end();
		 it++)
	{
		const std::string& uniformName = it->first;
		const std::string& dataSourceName = it->second;

		glm::mat4 mat4Value;
		if (m_mat4Sources.tryGetValue(dataSourceName, mat4Value))
		{
			bSuccess = material->setMat4ByUniformName(uniformName, mat4Value);
		}
		else
		{
			bSuccess = false;
		}
	}

	return bSuccess;
}

bool GlFrameCompositor::applyLayerMaterialTextures(
	const CompositorLayerConfig& layerConfig,
	GlFrameCompositor::Layer& layer)
{
	bool bSuccess= true;

	GlMaterial* material= layer.layerMaterial;
	for (auto it = layerConfig.shaderConfig.colorTextureSourceMap.begin();
		 it != layerConfig.shaderConfig.colorTextureSourceMap.end();
		 it++)
	{
		const std::string& uniformName= it->first;
		const std::string& dataSourceName= it->second;

		GlTexture* dataSourceTexture;
		if (m_colorTextureSources.tryGetValue(dataSourceName, dataSourceTexture) && dataSourceTexture != nullptr)
		{
			bSuccess= material->setTextureByUniformName(uniformName, dataSourceTexture);
		}
		else
		{
			bSuccess= false;
		}
	}

	return bSuccess;
}

const GlRenderModelResource* GlFrameCompositor::getStencilRenderModel(MikanStencilID stencilId) const
{
	auto it = m_stencilMeshCache.find(stencilId);

	if (it != m_stencilMeshCache.end())
	{
		return it->second;
	}

	return nullptr;
}

void GlFrameCompositor::flushStencilRenderModel(MikanStencilID stencilId)
{
	auto it = m_stencilMeshCache.find(stencilId);

	if (it != m_stencilMeshCache.end())
	{
		// updateStencils() will reload the meshes if the model path is still valid for this stencil
		m_stencilMeshCache.erase(it);
	}
}

void GlFrameCompositor::updateQuadStencils(
	const CompositorQuadStencilLayerConfig& stencilConfig,
	GlState* glState)
{
	EASY_FUNCTION();

	// Bail if we don't want any of this kind of stencil
	if (stencilConfig.stencilMode == eCompositorStencilMode::noStencil)
		return;

	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	// Can't apply stencils unless we have a valid tracked camera pose
	glm::mat4 cameraXform;
	if (!getVideoSourceCameraPose(cameraXform))
		return;

	// Collect stencil in view of the tracked camera
	const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
	const glm::vec3 cameraPosition(cameraXform[3]);

	std::vector<const MikanStencilQuad*> quadStencilList;
	MikanServer::getInstance()->getRelevantQuadStencilList(
		&stencilConfig.quadStencilNames,
		cameraPosition, 
		cameraForward, 
		quadStencilList);

	if (quadStencilList.size() == 0)
		return;

	// If the camera is behind all double sided stencil quads
	// reverse the stencil function so that video is drawn inside the stencil.
	// This makes the stencils act like a magic portal into the virtual layers.
	bool bInvertStencils = false;
	if (stencilConfig.bInvertWhenCameraInside)
	{
		int cameraBehindStencilCount = 0;
		int doubleSidedStencilCount = 0;
		for (const MikanStencilQuad* stencil : quadStencilList)
		{
			if (!stencil->is_disabled && stencil->is_double_sided)
			{
				const glm::mat4 xform = profileConfig->getQuadStencilWorldTransform(stencil);
				const glm::vec3 quadCenter = glm::vec3(xform[3]);
				const glm::vec3 quadNormal = glm::vec3(xform[2]);
				const glm::vec3 cameraToQuadCenter = quadCenter - cameraPosition;

				if (glm::dot(quadNormal, cameraToQuadCenter) > 0.f)
				{
					cameraBehindStencilCount++;
				}

				doubleSidedStencilCount++;
			}
		}
		bInvertStencils =
			cameraBehindStencilCount > 0 &&
			cameraBehindStencilCount == doubleSidedStencilCount;
	}

	glClearStencil(0);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Do not draw any pixels on the back buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	// Enables testing AND writing functionalities
	glState->enableFlag(eGlStateFlagType::stencilTest);
	// Do not test the current value in the stencil buffer, always accept any value on there for drawing
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	// Make every test succeed
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	// Compute the view-projection matrix for the tracked video source
	const glm::mat4 vpMatrix = m_videoSourceView->getCameraViewProjectionMatrix(m_cameraTrackingPuckView);

	m_stencilShader->bindProgram();

	// Draw stencil quads first
	for (const MikanStencilQuad* stencil : quadStencilList)
	{
		// Set the model matrix of stencil quad
		const glm::mat4 xform = profileConfig->getQuadStencilWorldTransform(stencil);
		const glm::vec3 x_axis = glm::vec3(xform[0]) * stencil->quad_width;
		const glm::vec3 y_axis = glm::vec3(xform[1]) * stencil->quad_height;
		const glm::vec3 z_axis = glm::vec3(xform[2]);
		const glm::vec3 position = glm::vec3(xform[3]);
		const glm::mat4 modelMatrix =
			glm::mat4(
				glm::vec4(x_axis, 0.f),
				glm::vec4(y_axis, 0.f),
				glm::vec4(z_axis, 0.f),
				glm::vec4(position, 1.f));

		// Set the model-view-projection matrix on the stencil shader
		m_stencilShader->setMatrix4x4Uniform(STENCIL_MVP_UNIFORM_NAME, vpMatrix * modelMatrix);

		glBindVertexArray(m_stencilQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	m_stencilShader->unbindProgram();

	// Make sure you will no longer (over)write stencil values, even if any test succeeds
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	// Make sure we draw on the backbuffer again.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	// Now we will only draw pixels where the corresponding stencil buffer value == (nor !=) 1
	GLenum stencilMode = (stencilConfig.stencilMode == eCompositorStencilMode::outsideStencil) ? GL_NOTEQUAL : GL_EQUAL;
	if (bInvertStencils)
	{
		// Flip the stencil mode from whatever the default was
		stencilMode= (stencilMode == GL_EQUAL) ? GL_NOTEQUAL : GL_EQUAL;
	}
	glStencilFunc(stencilMode, 1, 0xFF);
}

void GlFrameCompositor::updateBoxStencils(
	const CompositorBoxStencilLayerConfig& stencilConfig,
	GlState* glState)
{
	EASY_FUNCTION();

	// Bail if we don't want any of this kind of stencil
	if (stencilConfig.stencilMode == eCompositorStencilMode::noStencil)
		return;

	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	// Can't apply stencils unless we have a valid tracked camera pose
	glm::mat4 cameraXform;
	if (!getVideoSourceCameraPose(cameraXform))
		return;

	// Collect stencil in view of the tracked camera
	const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
	const glm::vec3 cameraPosition(cameraXform[3]);

	std::vector<const MikanStencilBox*> boxStencilList;
	MikanServer::getInstance()->getRelevantBoxStencilList(
		&stencilConfig.boxStencilNames,
		cameraPosition, 
		cameraForward, 
		boxStencilList);

	if (boxStencilList.size() == 0)
		return;

	glClearStencil(0);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Do not draw any pixels on the back buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	// Enables testing AND writing functionalities
	glState->enableFlag(eGlStateFlagType::stencilTest);
	// Do not test the current value in the stencil buffer, always accept any value on there for drawing
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	// Make every test succeed
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	// Compute the view-projection matrix for the tracked video source
	const glm::mat4 vpMatrix = m_videoSourceView->getCameraViewProjectionMatrix(m_cameraTrackingPuckView);

	m_stencilShader->bindProgram();

	// Then draw stencil boxes ...
	for (const MikanStencilBox* stencil : boxStencilList)
	{
		// Set the model matrix of stencil quad
		const glm::mat4 xform = profileConfig->getBoxStencilWorldTransform(stencil);
		const glm::vec3 x_axis = glm::vec3(xform[0]) * stencil->box_x_size;
		const glm::vec3 y_axis = glm::vec3(xform[1]) * stencil->box_y_size;
		const glm::vec3 z_axis = glm::vec3(xform[2]) * stencil->box_z_size;
		const glm::vec3 position = glm::vec3(xform[3]);
		const glm::mat4 modelMatrix =
			glm::mat4(
				glm::vec4(x_axis, 0.f),
				glm::vec4(y_axis, 0.f),
				glm::vec4(z_axis, 0.f),
				glm::vec4(position, 1.f));

		// Set the model-view-projection matrix on the stencil shader
		m_stencilShader->setMatrix4x4Uniform(STENCIL_MVP_UNIFORM_NAME, vpMatrix * modelMatrix);

		glBindVertexArray(m_stencilBoxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
		glBindVertexArray(0);
	}

	m_stencilShader->unbindProgram();

	// Make sure you will no longer (over)write stencil values, even if any test succeeds
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	// Make sure we draw on the backbuffer again.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	// Now we will only draw pixels where the corresponding stencil buffer value == (nor !=) 1
	const GLenum stencilMode = (stencilConfig.stencilMode == eCompositorStencilMode::outsideStencil) ? GL_NOTEQUAL : GL_EQUAL;
	glStencilFunc(stencilMode, 1, 0xFF);
}

void GlFrameCompositor::updateModelStencils(
	const CompositorModelStencilLayerConfig& stencilConfig,
	GlState* glState)
{
	EASY_FUNCTION();

	// Bail if we don't want any of this kind of stencil
	if (stencilConfig.stencilMode == eCompositorStencilMode::noStencil)
		return;

	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();
	std::unique_ptr<class GlModelResourceManager>& modelResourceManager = Renderer::getInstance()->getModelResourceManager();

	std::vector<const MikanStencilModelConfig*> modelStencilList;
	MikanServer::getInstance()->getRelevantModelStencilList(
		&stencilConfig.modelStencilNames,
		modelStencilList);

	if (modelStencilList.size() == 0)
		return;

	// Add any missing stencil models to the model cache
	for (const MikanStencilModelConfig* modelConfig : modelStencilList)
	{
		const MikanStencilID stencil_id = modelConfig->modelInfo.stencil_id;

		if (m_stencilMeshCache.find(stencil_id) == m_stencilMeshCache.end())
		{
			// It's possible that the model path isn't valid, 
			// in which case renderModelResource will be null.
			// Go ahead an occupy a slot in the m_stencilMeshCache until
			// the entry us explicitly cleared by flushStencilRenderModel.
			GlRenderModelResource* renderModelResource =
				modelResourceManager->fetchRenderModel(
					modelConfig->modelPath,
					getStencilModelVertexDefinition());

			m_stencilMeshCache.insert({stencil_id, renderModelResource});
		}
	}

	glClearStencil(0);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Do not draw any pixels on the back buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	// Enables testing AND writing functionalities
	glState->enableFlag(eGlStateFlagType::stencilTest);
	// Do not test the current value in the stencil buffer, always accept any value on there for drawing
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
	// Make every test succeed
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	// Compute the view-projection matrix for the tracked video source
	const glm::mat4 vpMatrix = m_videoSourceView->getCameraViewProjectionMatrix(m_cameraTrackingPuckView);

	m_stencilShader->bindProgram();

	// Then draw stencil models
	for (const MikanStencilModelConfig* modelConfig : modelStencilList)
	{
		const MikanStencilID stencil_id = modelConfig->modelInfo.stencil_id;
		auto it = m_stencilMeshCache.find(stencil_id);

		if (it != m_stencilMeshCache.end())
		{
			GlRenderModelResource* renderModelResource = it->second;

			if (renderModelResource != nullptr)
			{
				// Set the model matrix of stencil model
				const glm::mat4 modelMatrix = profileConfig->getModelStencilWorldTransform(&modelConfig->modelInfo);

				// Set the model-view-projection matrix on the stencil shader
				m_stencilShader->setMatrix4x4Uniform(STENCIL_MVP_UNIFORM_NAME, vpMatrix * modelMatrix);

				for (int meshIndex = 0; meshIndex < (int)renderModelResource->getTriangulatedMeshCount(); ++meshIndex)
				{
					const GlTriangulatedMesh* mesh = renderModelResource->getTriangulatedMesh(meshIndex);

					mesh->drawElements();
				}
			}
		}
	}

	m_stencilShader->unbindProgram();

	// Make sure you will no longer (over)write stencil values, even if any test succeeds
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	// Make sure we draw on the backbuffer again.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	// Now we will only draw pixels where the corresponding stencil buffer value == (nor !=) 1
	const GLenum stencilMode = (stencilConfig.stencilMode == eCompositorStencilMode::outsideStencil) ? GL_NOTEQUAL : GL_EQUAL;
	glStencilFunc(stencilMode, 1, 0xFF);
}

void GlFrameCompositor::render() const
{
	if (!getIsRunning())
		return;

	if (m_compositedFrame != nullptr)
	{
		GlScopedState scopedState= Renderer::getInstance()->getGlStateStack()->createScopedState();
		scopedState.getStackState().disableFlag(eGlStateFlagType::depthTest);

		// Draw the composited video frame
		m_rgbFrameShader->bindProgram();
		m_compositedFrame->bindTexture();
		glBindVertexArray(m_layerQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		m_compositedFrame->clearTexture();
		m_rgbFrameShader->unbindProgram();
	}
}

bool GlFrameCompositor::openVideoSource()
{
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

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

		// Add the video textures to the color texture data source table, 
		// but initially mark as invalid until we read in a video frame
		m_colorTextureSources.setValue("videoTexture", nullptr);
		m_colorTextureSources.setValue("distortionTexture", nullptr);
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
	// Remove the video texture entries from the color texture data source table
	m_colorTextureSources.removeValue("videoTexture");
	m_colorTextureSources.removeValue("distortionTexture");

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
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	m_cameraTrackingPuckView= VRDeviceListIterator(eDeviceType::VRTracker, profileConfig->cameraVRDevicePath).getCurrent();

	return m_cameraTrackingPuckView != nullptr;
}

std::string GlFrameCompositor::makeClientRendererTextureName(int clientSourceIndex)
{
	// Use standard naming based on client connection order 
	// so that we can refer to the render texture data source in GlFrameCompositorConfig templates
	char dataSourceName[32];
	StringUtils::formatString(
		dataSourceName, sizeof(dataSourceName),
		"clientRenderTexture_%d", clientSourceIndex);

	return dataSourceName;
}

std::string GlFrameCompositor::makeClientColorKeyName(int clientSourceIndex)
{
	// Use standard naming based on client connection order 
	// so that we can refer to the color key data source in GlFrameCompositorConfig templates
	char dataSourceName[32];
	StringUtils::formatString(
		dataSourceName, sizeof(dataSourceName),
		"clientColorKey_%d", clientSourceIndex);

	return dataSourceName;
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
		clientSource->colorTexture = new GlTexture();
		clientSource->colorTexture->setTextureFormat(GL_RGB);
		clientSource->colorTexture->setBufferFormat(GL_RGB);
		break;
	case MikanColorBuffer_RGBA32:
		clientSource->colorTexture = new GlTexture();
		clientSource->colorTexture->setTextureFormat(GL_RGBA);
		clientSource->colorTexture->setBufferFormat(GL_RGBA);
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
		clientSource->depthTexture = new GlTexture();
		clientSource->depthTexture->setTextureFormat(GL_DEPTH_COMPONENT16);
		clientSource->depthTexture->setBufferFormat(GL_DEPTH_COMPONENT);
		break;
	case MikanDepthBuffer_DEPTH32:
		clientSource->depthTexture = new GlTexture();
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

	// If the client source has a valid color texture
	// add it to the color texture data source table
	if (clientSource->colorTexture != nullptr)
	{
		const std::string colorTextureSourceName = makeClientRendererTextureName(clientSource->clientSourceIndex);
		m_colorTextureSources.setValue(colorTextureSourceName, clientSource->colorTexture);

		const glm::vec3 color_key(desc.color_key.r, desc.color_key.g, desc.color_key.b);
		const std::string colorKeySourceName = makeClientColorKeyName(clientSource->clientSourceIndex);
		m_float3Sources.setValue(colorKeySourceName, color_key);
	}

	return true;
}

bool GlFrameCompositor::removeClientSource(
	const std::string& clientId,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	GlFrameCompositor::ClientSource* clientSource = m_clientSources.getValueOrDefault(clientId, nullptr);
	if (clientSource == nullptr)
		return false;

	if (clientSource->colorTexture != nullptr)
	{
		clientSource->colorTexture->disposeTexture();
		delete clientSource->colorTexture;

		readAccessor->setColorTexture(nullptr);
	}

	if (clientSource->depthTexture != nullptr)
	{
		clientSource->depthTexture->disposeTexture();
		delete clientSource->depthTexture;

		readAccessor->setDepthTexture(nullptr);
	}

	// Remove the client source entries from the data source tables
	m_clientSources.removeValue(clientId);
	m_colorTextureSources.removeValue(clientId);

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
	m_compositedFrame =
		(new GlTexture())
		->setSize(width, height)
		->setTextureFormat(GL_RGB)
		->setBufferFormat(GL_RGB)
		->setGenerateMipMap(false);
	m_compositedFrame->createTexture();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_compositedFrame->getGlTextureId(), 0);

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

	if (m_compositedFrame != nullptr)
	{
		m_compositedFrame->disposeTexture();
		delete m_compositedFrame;
		m_compositedFrame = nullptr;
	}

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
	m_bgrVideoFrame =
		(new GlTexture())
		->setSize(width, height)
		->setTextureFormat(GL_RGB)
		->setBufferFormat(GL_RGB)
		->setGenerateMipMap(false)
		->setPixelBufferObjectMode(GlTexture::PixelBufferObjectMode::DoublePBORead);
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

	if (m_bgrVideoFrame != nullptr)
	{
		m_bgrVideoFrame->disposeTexture();
		delete m_bgrVideoFrame;
		m_bgrVideoFrame = nullptr;
	}

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

	// vertex attributes that represents a 3d scaled stencil quad
	const float stencilQuadVertices[] = {
		// positions
		-0.5f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,

		-0.5f,  0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
	};

	// vertex attributes that represents a 3d scaled stencil box
	const float stencilBoxVertices[] = {
		// positions
		-0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,

		0.5f, 0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,
		-0.5f, 0.5f,-0.5f,

		0.5f,-0.5f, 0.5f,
		-0.5f,-0.5f,-0.5f,
		0.5f,-0.5f,-0.5f,

		0.5f, 0.5f,-0.5f,
		0.5f,-0.5f,-0.5f,
		-0.5f,-0.5f,-0.5f,

		-0.5f,-0.5f,-0.5f,
		-0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f,-0.5f,

		0.5f,-0.5f, 0.5f,
		-0.5f,-0.5f, 0.5f,
		-0.5f,-0.5f,-0.5f,

		-0.5f, 0.5f, 0.5f,
		-0.5f,-0.5f, 0.5f,
		0.5f,-0.5f, 0.5f,

		0.5f, 0.5f, 0.5f,
		0.5f,-0.5f,-0.5f,
		0.5f, 0.5f,-0.5f,

		0.5f,-0.5f,-0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f,-0.5f, 0.5f,

		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f,-0.5f,
		-0.5f, 0.5f,-0.5f,

		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f,-0.5f,
		-0.5f, 0.5f, 0.5f,

		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f,-0.5f, 0.5f
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

	// stencil quad VAO/VBO
	glGenVertexArrays(1, &m_stencilQuadVAO);
	glGenBuffers(1, &m_stencilQuadVBO);
	glBindVertexArray(m_stencilQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_stencilQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(stencilQuadVertices), &stencilQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// stencil box VAO/VBO
	glGenVertexArrays(1, &m_stencilBoxVAO);
	glGenBuffers(1, &m_stencilBoxVBO);
	glBindVertexArray(m_stencilBoxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_stencilBoxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(stencilBoxVertices), &stencilBoxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
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

	if (m_stencilQuadVAO != 0)
	{
		glDeleteVertexArrays(1, &m_stencilQuadVAO);
		m_stencilQuadVAO = 0;
	}
	if (m_stencilQuadVBO != 0)
	{
		glDeleteBuffers(1, &m_stencilQuadVBO);
		m_stencilQuadVBO = 0;
	}

	if (m_stencilBoxVAO != 0)
	{
		glDeleteVertexArrays(1, &m_stencilBoxVAO);
		m_stencilBoxVAO = 0;
	}
	if (m_stencilBoxVBO != 0)
	{
		glDeleteBuffers(1, &m_stencilBoxVBO);
		m_stencilBoxVBO = 0;
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

const GlProgramCode* GlFrameCompositor::getStencilShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"Internal Stencil Shader Code",
		// vertex shader
		R""""(
			#version 330 core
			layout (location = 0) in vec3 aPos;

			uniform mat4 mvpMatrix;

			void main()
			{
				gl_Position = mvpMatrix * vec4(aPos, 1.0);
			}
			)"""",
		//fragment shader
		R""""(
			#version 330 core
			out vec4 FragColor;

			void main()
			{    
				FragColor = vec4(1, 1, 1, 1);
			}
			)"""")
		.addUniform(STENCIL_MVP_UNIFORM_NAME, eUniformSemantic::modelViewProjectionMatrix);

	return &x_shaderCode;
}

const GlVertexDefinition* GlFrameCompositor::getStencilModelVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const int32_t vertexSize = (int32_t)sizeof(float)*3;
		std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

		attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position3f, false, vertexSize, 0));

		x_vertexDefinition.vertexSize = vertexSize;
	}

	return &x_vertexDefinition;
}