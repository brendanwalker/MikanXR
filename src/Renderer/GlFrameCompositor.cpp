#include "App.h"
#include "GlCommon.h"
#include "GlFrameCompositor.h"
#include "GlTexture.h"
#include "GlTextRenderer.h"
#include "InterprocessRenderTargetReader.h"
#include "Logger.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "StringUtils.h"
#include "GlShaderCache.h"
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

const std::string g_compositorLayerAlphaStrings[(int)eCompositorLayerAlphaMode::COUNT] = {
	"NoAlpha",
	"ColorKey",
	"AlphaChannel",
	"MagicPortal"
};
const std::string* k_compositorLayerAlphaStrings = g_compositorLayerAlphaStrings;

// -- CompositorLayerConfig ------
const configuru::Config CompositorLayerConfig::writeToJSON() const
{
	configuru::Config pt{
		{"appName", appName},
		{"alphaMode", g_compositorLayerAlphaStrings[(int)eCompositorLayerAlphaMode::NoAlpha]},
	};

	return pt;
}

void CompositorLayerConfig::readFromJSON(const configuru::Config& pt)
{
	appName = pt.get_or<std::string>("appName", appName);

	std::string alphaModeString = 
		pt.get_or<std::string>(
			"alphaMode", 
			g_compositorLayerAlphaStrings[(int)eCompositorLayerAlphaMode::NoAlpha]);
	alphaMode =
		StringUtils::FindEnumValue<eCompositorLayerAlphaMode>(
			alphaModeString,
			g_compositorLayerAlphaStrings);
}

// -- GlFrameCompositorConfig ------
const configuru::Config GlFrameCompositorConfig::writeToJSON()
{
	configuru::Config pt= configuru::Config::object();

	// Write out the layers
	std::vector<configuru::Config> layerConfigs;
	for (const CompositorLayerConfig& layer : layers)
	{
		layerConfigs.push_back(layer.writeToJSON());
	}
	pt.insert_or_assign(std::string("layers"), layerConfigs);

	return pt;
}

void GlFrameCompositorConfig::readFromJSON(
	const configuru::Config& pt)
{
	layers.clear();
	if (pt.has_key("layers"))
	{
		for (const configuru::Config& layerConfig : pt["layers"].as_array())
		{
			CompositorLayerConfig layer;

			layer.readFromJSON(layerConfig);
			layers.push_back(layer);
		}
	}
}

const CompositorLayerConfig& GlFrameCompositorConfig::findOrAddDefaultLayerConfig(
	const MikanClientInfo& clientInfo,
	const MikanRenderTargetDescriptor& renderTargetDesc)
{
	// Key layer config off of the appName
	const std::string appName= clientInfo.applicationName;

	auto it = std::find_if(
		layers.begin(), layers.end(),
		[appName](const CompositorLayerConfig& layerConfig) {
		return layerConfig.appName == appName;
	});

	if (it == layers.end())
	{
		CompositorLayerConfig layerConfig;
		layerConfig.appName= clientInfo.applicationName;
	
		if (renderTargetDesc.color_buffer_type == MikanColorBuffer_RGBA32)
		{
			layerConfig.alphaMode = eCompositorLayerAlphaMode::AlphaChannel;
		}
		else
		{
			layerConfig.alphaMode = eCompositorLayerAlphaMode::ColorKey;
		}

		layers.push_back(layerConfig);
		save();

		return layers[layers.size() - 1];
	}
	else
	{
		return *it;
	}
}

CompositorLayerConfig* GlFrameCompositorConfig::findLayerConfig(const MikanClientInfo& clientInfo)
{
	// Key layer config off of the appName
	const std::string appName = clientInfo.applicationName;

	auto it = std::find_if(
		layers.begin(), layers.end(),
		[appName](const CompositorLayerConfig& layerConfig) {
		return layerConfig.appName == appName;
	});

	if (it != layers.end())
	{
		return &(*it);
	}

	return nullptr;
}

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

	m_rgbUndistortionFrameShader = GlShaderCache::getInstance()->fetchCompiledGlProgram(getRGBUndistortionFrameShaderCode());
	if (m_rgbUndistortionFrameShader == nullptr)
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile rgb undistortion frame shader";
		return false;
	}

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

	m_rgbColorKeyLayerShader= GlShaderCache::getInstance()->fetchCompiledGlProgram(getRGBColorKeyLayerShaderCode());
	if (m_rgbColorKeyLayerShader == nullptr)
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile RGB color key shader";
		return false;
	}

	m_rgbaLayerShader= GlShaderCache::getInstance()->fetchCompiledGlProgram(getRGBALayerShaderCode());
	if (m_rgbaLayerShader == nullptr)
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile RGBA layer shader";
		return false;
	}

	m_stencilShader = GlShaderCache::getInstance()->fetchCompiledGlProgram(getStencilShaderCode());
	if (m_stencilShader == nullptr)
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile stencil shader";
		return false;
	}

	createVertexBuffers();

	m_config.load();

	return true;
}

void GlFrameCompositor::shutdown()
{
	stop();
	freeVertexBuffers();
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
	// Clean up any allocated layers
	for (GlFrameCompositor::Layer& layer : m_layers)
	{
		if (layer.colorTexture != nullptr)
		{
			layer.colorTexture->disposeTexture();
			delete layer.colorTexture;
		}

		if (layer.depthTexture != nullptr)
		{
			layer.depthTexture->disposeTexture();
			delete layer.depthTexture;
		}
	}
	m_layers.clear();

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
		size_t layerReadyCount = 0;
		for (GlFrameCompositor::Layer& layer : m_layers)
		{
			if (!layer.bIsPendingRender)
			{
				layerReadyCount++;
			}
		}

		// If the video frame and layers are fresh, composite them together
		if (layerReadyCount == m_layers.size())
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

		// Mark all layers as pending
		for (GlFrameCompositor::Layer& layer : m_layers)
		{
			layer.bIsPendingRender= true;
		}

		// Track the index of the pending frame
		m_pendingCompositeFrameIndex = newFrameEvent.frame;

		// Tell all clients that we have a new frame to render
		MIKAN_LOG_TRACE("GlFrameCompositor::update") << "Send frame " << m_pendingCompositeFrameIndex;
		MikanServer::getInstance()->publishNewVideoFrameEvent(newFrameEvent);
	}
}

void GlFrameCompositor::updateCompositeFrame()
{
	EASY_FUNCTION();

	assert(m_pendingCompositeFrameIndex != 0);

	// Cache the last viewport dimensions
	GLint last_viewport[4];

	{
		EASY_BLOCK("Begin Composite")

		glGetIntegerv(GL_VIEWPORT, last_viewport);

		// Change the viewport to match the frame buffer texture
		glViewport(0, 0, m_compositedFrame->getTextureWidth(), m_compositedFrame->getTextureHeight());

		// bind to framebuffer and draw scene as we normally would to color texture 
		glBindFramebuffer(GL_FRAMEBUFFER, m_layerFramebuffer);

		// Turn off depth testing for compositing
		glDisable(GL_DEPTH_TEST);

		// Turn off blending for the base layer
		glDisable(GL_BLEND);

		// make sure we clear the framebuffer's content
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	// Read the next queued frame into a texture to composite
	if (m_videoDistortionView->processVideoFrame(m_pendingCompositeFrameIndex))
	{
		EASY_BLOCK("Render Video Frame")

		// Draw the base video frame first
		m_rgbUndistortionFrameShader->bindProgram();

		// Bind the distorted frame texture + un-distortion textures
		GlTexture* videoTexture = m_videoDistortionView->getVideoTexture();
		GlTexture* distortionTexture = m_videoDistortionView->getDistortionTexture();
		if (videoTexture && distortionTexture)
		{
			m_rgbUndistortionFrameShader->setTextureUniform(eUniformSemantic::texture0);
			m_rgbUndistortionFrameShader->setTextureUniform(eUniformSemantic::texture1);

			videoTexture->bindTexture(0);
			distortionTexture->bindTexture(1);

			glBindVertexArray(m_videoQuadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);

			videoTexture->clearTexture(0);
			distortionTexture->clearTexture(1);
		}
		m_rgbUndistortionFrameShader->unbindProgram();
	}

	// Turn back on blending for compositing
	// https://www.andersriggelsen.dk/glblendfunc.php
	// (sR*sA) + (dR*(1-sA)) = rR
	// (sG*sA) + (dG*(1-sA)) = rG
	// (sB*sA) + (dB*(1-sA)) = rB
	// (sA*sA) + (dA*(1-sA)) = rA
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	{
		EASY_BLOCK("Render Layers")

		// Draw each of the layers next
		for (GlFrameCompositor::Layer& layer : m_layers)
		{
			// Render the color buffer
			switch (layer.alphaMode)
			{
				case eCompositorLayerAlphaMode::NoAlpha:
				{
					// Render the stencil
					bool bIsUsingStencils = updateStencils();

					m_rgbFrameShader->bindProgram();
					layer.colorTexture->bindTexture();

					glBindVertexArray(m_layerQuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glBindVertexArray(0);

					layer.colorTexture->clearTexture();
					m_rgbFrameShader->unbindProgram();

					// Turn back off stencils if we enabled them
					if (bIsUsingStencils)
					{
						glDisable(GL_STENCIL_TEST);
					}
				} break;
			case eCompositorLayerAlphaMode::ColorKey:
				{
					// Render the stencil
					bool bIsUsingStencils = updateStencils();

					const MikanColorRGB& colorKey = layer.desc.color_key;
					const glm::vec3 glmColorKey(colorKey.r, colorKey.g, colorKey.b);

					m_rgbColorKeyLayerShader->bindProgram();
					m_rgbColorKeyLayerShader->setVector3Uniform(eUniformSemantic::diffuseColorRGB, glmColorKey);
					layer.colorTexture->bindTexture();

					glBindVertexArray(m_layerQuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glBindVertexArray(0);

					layer.colorTexture->clearTexture();
					m_rgbColorKeyLayerShader->unbindProgram();

					// Turn back off stencils if we enabled them
					if (bIsUsingStencils)
					{
						glDisable(GL_STENCIL_TEST);
					}
				} break;
			case eCompositorLayerAlphaMode::AlphaChannel:
				{
					// Render the stencil
					bool bIsUsingStencils = updateStencils();

					m_rgbaLayerShader->bindProgram();
					layer.colorTexture->bindTexture();

					glBindVertexArray(m_layerQuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glBindVertexArray(0);

					layer.colorTexture->clearTexture();
					m_rgbaLayerShader->unbindProgram();

					// Turn back off stencils if we enabled them
					if (bIsUsingStencils)
					{
						glDisable(GL_STENCIL_TEST);
					}
				} break;
			case eCompositorLayerAlphaMode::MagicPortal:
				{
					// Render the stencil
					bool bIsUsingStencils = updateStencils();

					// Render the no-alpha frame first (presumably stenciled out)
					m_rgbFrameShader->bindProgram();
					layer.colorTexture->bindTexture();

					glBindVertexArray(m_layerQuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glBindVertexArray(0);

					layer.colorTexture->clearTexture();
					m_rgbFrameShader->unbindProgram();

					// Turn back off stencils if we enabled them
					if (bIsUsingStencils)
					{
						glDisable(GL_STENCIL_TEST);
					}

					// Render the alpha frame second (without the stencil
					m_rgbaLayerShader->bindProgram();
					layer.colorTexture->bindTexture();

					glBindVertexArray(m_layerQuadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					glBindVertexArray(0);

					layer.colorTexture->clearTexture();
					m_rgbaLayerShader->unbindProgram();

				}
			}

			// Render the depth buffer
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
		m_rgbToBgrFrameShader->setTextureUniform(eUniformSemantic::texture0);

		m_compositedFrame->bindTexture(0);

		glBindVertexArray(m_videoQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		m_compositedFrame->clearTexture(0);

		m_rgbToBgrFrameShader->unbindProgram();

		// unbind the bghr frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Turn back on depth testing for compositing
	glEnable(GL_DEPTH_TEST);


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

bool GlFrameCompositor::updateStencils()
{
	EASY_FUNCTION();

	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();	
	std::unique_ptr<class GlModelResourceManager>& modelResourceManager= Renderer::getInstance()->getModelResourceManager();

	// Can't apply stencils unless we have a valid tracked camera pose
	glm::mat4 cameraXform;
	if (!getVideoSourceCameraPose(cameraXform))
		return false;

	// Collect stencil in view of the tracked camera
	const glm::vec3 cameraForward(cameraXform[2] * -1.f); // Camera forward is along negative z-axis
	const glm::vec3 cameraPosition(cameraXform[3]);

	std::vector<const MikanStencilQuad*> quadStencilList;
	MikanServer::getInstance()->getRelevantQuadStencilList(cameraPosition, cameraForward, quadStencilList);

	std::vector<const MikanStencilModelConfig*> modelStencilList;
	MikanServer::getInstance()->getRelevantModelStencilList(modelStencilList);

	if (quadStencilList.size() == 0 && modelStencilList.size() == 0)
		return false;

	// Add any missing stencil models to the model cache
	for (const MikanStencilModelConfig* modelConfig : modelStencilList)
	{		
		const MikanStencilID stencil_id= modelConfig->modelInfo.stencil_id;

		if (m_stencilMeshCache.find(stencil_id) == m_stencilMeshCache.end())
		{
			// It's possible that the model path isn't valid, 
			// in which case renderModelResource will be null.
			// Go ahead an occupy a slot in the m_stencilMeshCache until
			// the entry us explicitly cleared by flushStencilRenderModel.
			GlRenderModelResource* renderModelResource=
				modelResourceManager->fetchRenderModel(
					modelConfig->modelPath, 
					getStencilModelVertexDefinition());

			m_stencilMeshCache.insert({ stencil_id, renderModelResource });
		}
	}

	// If the camera is behind all double sided stencil quads
	// reverse the stencil function so that video is drawn inside the stencil.
	// This makes the stencils act like a magic portal into the virtual layers.
	int cameraBehindStencilCount= 0;
	int doubleSidedStencilCount= 0;
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
	const bool bIsCameraBehindAllStencilPlanes = 
		cameraBehindStencilCount > 0 && 
		cameraBehindStencilCount == doubleSidedStencilCount;

	glClearStencil(0);
	glStencilMask(0xFF);
	glClear(GL_STENCIL_BUFFER_BIT);

	// Do not draw any pixels on the back buffer
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); 
	glDepthMask(GL_FALSE);
	// Enables testing AND writing functionalities
	glEnable(GL_STENCIL_TEST); 
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
		const glm::mat4 modelMatrix= 
			glm::mat4(
				glm::vec4(x_axis, 0.f),
				glm::vec4(y_axis, 0.f),
				glm::vec4(z_axis, 0.f),
				glm::vec4(position, 1.f));

		// Set the model-view-projection matrix on the stencil shader
		m_stencilShader->setMatrix4x4Uniform(eUniformSemantic::modelViewProjectionMatrix, vpMatrix * modelMatrix);

		glBindVertexArray(m_stencilQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

	// Then draw stencil models
	for (const MikanStencilModelConfig* modelConfig : modelStencilList)
	{
		const MikanStencilID stencil_id = modelConfig->modelInfo.stencil_id;
		auto it= m_stencilMeshCache.find(stencil_id);

		if (it != m_stencilMeshCache.end())
		{
			GlRenderModelResource* renderModelResource = it->second;

			if (renderModelResource != nullptr)
			{
				// Set the model matrix of stencil model
				const glm::mat4 modelMatrix = profileConfig->getModelStencilWorldTransform(&modelConfig->modelInfo);

				// Set the model-view-projection matrix on the stencil shader
				m_stencilShader->setMatrix4x4Uniform(eUniformSemantic::modelViewProjectionMatrix, vpMatrix * modelMatrix);

				for (int meshIndex = 0; meshIndex < (int)renderModelResource->getTriangulatedMeshCount(); ++meshIndex)
				{
					const GlTriangulatedMesh* mesh= renderModelResource->getTriangulatedMesh(meshIndex);

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
	glStencilFunc(bIsCameraBehindAllStencilPlanes ? GL_NOTEQUAL : GL_EQUAL, 1, 0xFF); 

	return true;
}

void GlFrameCompositor::render() const
{
	if (!getIsRunning())
		return;

	if (m_compositedFrame != nullptr)
	{
		glDisable(GL_DEPTH_TEST);

		// Draw the composited video frame
		m_rgbFrameShader->bindProgram();
		m_compositedFrame->bindTexture();
		glBindVertexArray(m_layerQuadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		m_compositedFrame->clearTexture();
		m_rgbFrameShader->unbindProgram();

		glEnable(GL_DEPTH_TEST);
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
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	m_cameraTrackingPuckView= VRDeviceListIterator(eDeviceType::VRTracker, profileConfig->cameraVRDevicePath).getCurrent();

	return m_cameraTrackingPuckView != nullptr;
}

void GlFrameCompositor::addLayer(
	const std::string& clientId, 
	const MikanClientInfo& clientInfo,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	ProfileConfig* profile= App::getInstance()->getProfileConfig();

	GlFrameCompositor::Layer newLayer;
	memset(&newLayer, 0, sizeof(GlFrameCompositor::Layer));

	const MikanRenderTargetDescriptor& desc= readAccessor->getRenderTargetDescriptor();
	const CompositorLayerConfig& layerConfig = m_config.findOrAddDefaultLayerConfig(clientInfo, desc);
	newLayer.alphaMode = layerConfig.alphaMode;
	newLayer.clientInfo = clientInfo;
	newLayer.clientId = clientId;
	newLayer.desc = desc;
	newLayer.frameIndex = 0;
	newLayer.bIsPendingRender = false;

	switch (desc.color_buffer_type)
	{
	case MikanColorBuffer_RGB24:
		newLayer.colorTexture = new GlTexture();
		newLayer.colorTexture->setTextureFormat(GL_RGB);
		newLayer.colorTexture->setBufferFormat(GL_RGB);
		break;
	case MikanColorBuffer_RGBA32:
		newLayer.colorTexture = new GlTexture();
		newLayer.colorTexture->setTextureFormat(GL_RGBA);
		newLayer.colorTexture->setBufferFormat(GL_RGBA);
		break;
	}

	if (newLayer.colorTexture != nullptr)
	{
		newLayer.colorTexture->setSize(desc.width, desc.height);
		newLayer.colorTexture->setGenerateMipMap(false);
		newLayer.colorTexture->setPixelBufferObjectMode(
			desc.graphicsAPI == MikanClientGraphicsAPI_UNKNOWN 
			? GlTexture::PixelBufferObjectMode::DoublePBOWrite
			: GlTexture::PixelBufferObjectMode::NoPBO);
		newLayer.colorTexture->createTexture();

		readAccessor->setColorTexture(newLayer.colorTexture);
	}

	switch (desc.depth_buffer_type)
	{
	case MikanDepthBuffer_DEPTH16:
		newLayer.depthTexture = new GlTexture();
		newLayer.depthTexture->setTextureFormat(GL_DEPTH_COMPONENT16);
		newLayer.depthTexture->setBufferFormat(GL_DEPTH_COMPONENT);
		break;
	case MikanDepthBuffer_DEPTH32:
		newLayer.depthTexture = new GlTexture();
		newLayer.depthTexture->setTextureFormat(GL_DEPTH_COMPONENT32F);
		newLayer.depthTexture->setBufferFormat(GL_DEPTH_COMPONENT);
		break;
	}

	if (newLayer.depthTexture != nullptr)
	{
		newLayer.depthTexture->setSize(desc.width, desc.height);
		newLayer.depthTexture->setPixelBufferObjectMode(
			desc.graphicsAPI == MikanClientGraphicsAPI_UNKNOWN
			? GlTexture::PixelBufferObjectMode::DoublePBOWrite
			: GlTexture::PixelBufferObjectMode::NoPBO);
		newLayer.depthTexture->createTexture();

		readAccessor->setDepthTexture(newLayer.depthTexture);
	}

	m_layers.push_back(newLayer);
}

void GlFrameCompositor::removeLayer(
	const std::string& clientId,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	auto it =
		std::find_if(
			m_layers.begin(), m_layers.end(),
			[&clientId](const GlFrameCompositor::Layer& elem) {
		return elem.clientId == clientId;
	});

	if (it != m_layers.end())
	{
		GlFrameCompositor::Layer& layer = *it;

		if (layer.colorTexture != nullptr)
		{
			layer.colorTexture->disposeTexture();
			delete layer.colorTexture;

			readAccessor->setColorTexture(nullptr);
		}

		if (layer.depthTexture != nullptr)
		{
			layer.depthTexture->disposeTexture();
			delete layer.depthTexture;

			readAccessor->setDepthTexture(nullptr);
		}

		m_layers.erase(it);
	}
}

void GlFrameCompositor::cycleNextLayerAlphaMode(int layerIndex)
{
	if (layerIndex < 0 && layerIndex >= (int)m_layers.size())
		return;

	Layer& layer= m_layers[layerIndex];
	eCompositorLayerAlphaMode newAlphaMode = layer.alphaMode;
	for (int attempt = 0; attempt < (int)eCompositorLayerAlphaMode::COUNT; ++attempt)
	{
		newAlphaMode= (eCompositorLayerAlphaMode)(((int)newAlphaMode + 1) % (int)eCompositorLayerAlphaMode::COUNT);

		if (newAlphaMode == eCompositorLayerAlphaMode::NoAlpha)
			break;
		if (newAlphaMode == eCompositorLayerAlphaMode::ColorKey)
			break;
		if (newAlphaMode == eCompositorLayerAlphaMode::AlphaChannel && layer.desc.color_buffer_type == MikanColorBuffer_RGBA32)
			break;
		if (newAlphaMode == eCompositorLayerAlphaMode::MagicPortal && layer.desc.color_buffer_type == MikanColorBuffer_RGBA32)
			break;
	}

	if (newAlphaMode != layer.alphaMode)
	{
		layer.alphaMode= newAlphaMode;

		CompositorLayerConfig* layerConfig= m_config.findLayerConfig(layer.clientInfo);
		if (layerConfig != nullptr)
		{
			layerConfig->alphaMode = newAlphaMode;
			m_config.save();
		}
	}
}

std::string GlFrameCompositor::getLayerAlphaModeString(int layerIndex) const
{
	if (layerIndex >= 0 && layerIndex < (int)m_layers.size())
	{
		const Layer& layer = m_layers[layerIndex];

		if (layer.alphaMode != eCompositorLayerAlphaMode::INVALID)
		{
			return g_compositorLayerAlphaStrings[(int)layer.alphaMode];
		}
	}

	return "";
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

	// vertex attributes for quad that represents a 3d scaled stencil quad
	const float stencilQuadVertices[] = {
		// positions
		-0.5f,  0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,

		-0.5f,  0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f,
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
}

// MikanServer Events
void GlFrameCompositor::onClientRenderTargetAllocated(
	const std::string& clientId, 
	const MikanClientInfo& clientInfo,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	addLayer(clientId, clientInfo, readAccessor);
}

void GlFrameCompositor::onClientRenderTargetReleased(
	const std::string& clientId,
	InterprocessRenderTargetReadAccessor* readAccessor)
{
	removeLayer(clientId, readAccessor);
}

void GlFrameCompositor::onClientRenderTargetUpdated(
	const std::string& clientId, 
	uint64_t frameIndex)
{
	EASY_FUNCTION();

	MIKAN_LOG_TRACE("GlFrameCompositor::onClientRenderTargetUpdated") << "Recv frame " << frameIndex;

	auto it =
		std::find_if(
			m_layers.begin(), m_layers.end(),
			[&clientId](const GlFrameCompositor::Layer& elem) {
		return elem.clientId == clientId;
	});

	if (it != m_layers.end())
	{
		GlFrameCompositor::Layer& layer = *it;

		// Update the frame index
		layer.frameIndex = frameIndex;

		// Mark that the layer is no longer pending the render
		//assert(layer.bIsPendingRender);
		layer.bIsPendingRender = false;
	}
}

const GlProgramCode* GlFrameCompositor::getRGBUndistortionFrameShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"RGB Undistortion Frame Shader Code",
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
			uniform sampler2D distortion;

			void main()
			{
				vec2 offset = texture(distortion, TexCoords.xy).rg;
				vec3 col = texture(rgbTexture, offset).rgb;

				FragColor = vec4(col, 1.0);
			} 
			)"""")
		.addUniform("rgbTexture", eUniformSemantic::texture0)
		.addUniform("distortion", eUniformSemantic::texture1);

	return &x_shaderCode;
}

const GlProgramCode* GlFrameCompositor::getRGBFrameShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"RGB Frame Shader Code",
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
		"BGR Frame Shader Code",
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

const GlProgramCode* GlFrameCompositor::getRGBColorKeyLayerShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"RGBColorKey Shader Code",
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

			uniform vec3 colorKey;
			uniform sampler2D colorKeyTexture;

			void main()
			{
				vec3 col = texture(colorKeyTexture, TexCoords).rgb;
				float alpha= (col == colorKey) ? 0.0 : 1.0;

				FragColor = vec4(col, alpha);
			} 
			)"""")
		.addUniform("colorKey", eUniformSemantic::diffuseColorRGB)
		.addUniform("colorKeyTexture", eUniformSemantic::texture0);

	return &x_shaderCode;
}

const GlProgramCode* GlFrameCompositor::getRGBALayerShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"RGBA Layer Shader Code",
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

			uniform sampler2D rgbaTexture;

			void main()
			{
				FragColor = texture(rgbaTexture, TexCoords).rgba;
			} 
			)"""")
		.addUniform("rgbaTexture", eUniformSemantic::texture0);

	return &x_shaderCode;
}

const GlProgramCode* GlFrameCompositor::getStencilShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"Stencil Shader Code",
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
		.addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix);

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