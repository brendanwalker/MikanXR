//-- inludes -----
#include "App.h"
#include "ClientTextureSourceComponent.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CameraRequestHandler.h"
#include "CompositorObjectSystem.h"
#include "IMkShaderCache.h"
#include "IMkTriangulatedMesh.h"
#include "IMkWindow.h"
#include "TextureSourceSettings/AppStage_TextureSourceSettings.h"
#include "TextureSourceSettings/RmlModel_TextureSourceSettings.h"
#include "Shared/RmlModel_ClientTextureSourceComponent.h"
#include "Shared/RmlModel_SpoutTextureSourceComponent.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "MikanTextRenderer.h"
#include "MikanServer.h"
#include "MainWindow.h"
#include "MkMaterialInstance.h"
#include "MulticastDelegate.h"
#include "SpoutTextureSourceComponent.h"
#include "ProjectConfig.h"
#include "TextureSourceComponent.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----__
const char* AppStage_TextureSourceSettings::APP_STAGE_NAME = "TextureSourceSettings";

//-- public methods -----
AppStage_TextureSourceSettings::AppStage_TextureSourceSettings(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_TextureSourceSettings::APP_STAGE_NAME)
{
}

void AppStage_TextureSourceSettings::enter()
{
	AppStage::enter();

	TextureSourceComponentPtr textureSourceComponent= m_textureSourceComponent.lock();

	// Cache the camera component for use in rendering the video frame in the app stage
	m_cameraComponent = getObjectSystemOfType<CameraObjectSystem>()->getCameraById(m_cameraId);

	// Pause all compositor components while in the texture source settings stage
	getObjectSystemOfType<CompositorObjectSystem>()->setAllCompositorsPaused(true);

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init Data Models
		auto* TextureSourceSettingsModel = addRmlModel<RmlModel_TextureSourceSettings>();
		TextureSourceSettingsModel->init(context);
		TextureSourceSettingsModel->OnReturnEvent = MakeDelegate(this, &AppStage_TextureSourceSettings::onReturnEvent);

		m_clientTextureSourceComponentModel = addRmlModel<RmlModel_ClientTextureSourceComponent>();
		m_clientTextureSourceComponentModel->init(this);
		if (auto clientTextureSourceComponent =
			std::dynamic_pointer_cast<ClientTextureSourceComponent>(textureSourceComponent))
		{
			m_clientTextureSourceComponentModel->setComponent(clientTextureSourceComponent);
		}

		auto* spoutTextureSourceComponentModel = addRmlModel<RmlModel_SpoutTextureSourceComponent>();
		spoutTextureSourceComponentModel->init(this);
		if (auto spoutTextureSourceComponent =
			std::dynamic_pointer_cast<SpoutTextureSourceComponent>(textureSourceComponent))
		{
			spoutTextureSourceComponentModel->setComponent(spoutTextureSourceComponent);
		}

		// Load the Rml view for the settings
		m_TextureSourceSettingsView = addRmlDocument("texture_source_settings.rml");

		// Show the main project view by default
		m_TextureSourceSettingsView->Show();
		m_TextureSourceSettingsView->PullToFront();
	}

	// Create a meshes used to render the video frame
	m_fullscreenRGBQuad = createFullscreenQuadMesh(m_ownerWindow, true, false);
	m_fullscreenRGBAQuad = createFullscreenQuadMesh(m_ownerWindow, true, true);
	m_fullscreenDepthUnpackQuad = 
		createFullscreenQuadMesh(
			m_ownerWindow, 
			m_ownerWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_UNPACK_RGBA_DEPTH_TEXTURE),
			true);
}

void AppStage_TextureSourceSettings::update(float deltaSeconds)
{
	AppStage::update(deltaSeconds);

	// Manually publish fake camera frame events that emulate new video frames.
	// This forces a connected client to render new frames.
	CameraComponentPtr cameraComponent = m_cameraComponent.lock();
	if (cameraComponent)
	{
		if (m_newFrameTimer <= 0.f)
		{
			if (MikanCameraNewFrameEvent newFrameEvent;
				cameraComponent->makeNewCameraFrameEvent(
					-1, // Skip video frame index
					1280, 720, // fallback render target size
					newFrameEvent))
			{
				getOwnerWindow()
					->getMikanServer()
					->getCameraRequestHandler()
					->publishCameraNewFrameEvent(newFrameEvent);
			}

			m_newFrameTimer = k_newFrameTimerDuration;
		}
		else
		{
			m_newFrameTimer -= deltaSeconds;
		}
	}
}

void AppStage_TextureSourceSettings::render(IMkViewportPtr targetViewport)
{
	AppStage::render(targetViewport);

	TextureSourceComponentPtr textureSourceComponent = m_textureSourceComponent.lock();

	IMkTriangulatedMeshPtr fullscreenQuad;
	IMkTexturePtr videoTexture;
	eUniformSemantic videoTextureSemantic = eUniformSemantic::INVALID;

	eTextureSourceDisplayBufferType displayBufferType = eTextureSourceDisplayBufferType::Color;
	if (m_clientTextureSourceComponentModel->getComponent() != nullptr)
	{
		displayBufferType = m_clientTextureSourceComponentModel->getDisplayBufferType();
	}
	
	switch (displayBufferType)
	{
		case eTextureSourceDisplayBufferType::Color:
		{
			videoTexture = textureSourceComponent->getClientColorSourceTexture(m_cameraId, eTextureSourceColorType::colorRGBA);
			if (videoTexture)
			{
				fullscreenQuad = m_fullscreenRGBAQuad;
				videoTextureSemantic = eUniformSemantic::rgbaTexture;
			}
			else
			{
				videoTexture = textureSourceComponent->getClientColorSourceTexture(m_cameraId, eTextureSourceColorType::colorRGB);
				if (videoTexture)
				{
					fullscreenQuad = m_fullscreenRGBQuad;
					videoTextureSemantic = eUniformSemantic::rgbTexture;
				}
			}
		} break;

		case eTextureSourceDisplayBufferType::Depth:
		{
			videoTexture = textureSourceComponent->getClientDepthSourceTexture(m_cameraId, eTextureSourceDepthType::depthPackRGBA);
			if (videoTexture)
			{
				fullscreenQuad = m_fullscreenDepthUnpackQuad;
				videoTextureSemantic = eUniformSemantic::rgbaTexture;
			}
		} break;
	};

	if (videoTexture && fullscreenQuad)
	{
		MkMaterialInstancePtr materialInstance = fullscreenQuad->getMaterialInstance();
		MkMaterialConstPtr material = materialInstance->getMaterial();

		if (auto materialBinding = material->bindMaterial())
		{
			// Bind the color texture
			materialInstance->setTextureBySemantic(videoTextureSemantic, videoTexture);

			// Draw the color texture
			if (auto materialInstanceBinding = materialInstance->bindMaterialInstance(materialBinding))
			{
				fullscreenQuad->drawElements();
			}
		}
	}
}

void AppStage_TextureSourceSettings::exit()
{
	// Resume all compositor components when exiting the texture source settings stage
	getObjectSystemOfType<CompositorObjectSystem>()->setAllCompositorsPaused(false);

	AppStage::exit();
}

void AppStage_TextureSourceSettings::onReturnEvent()
{
	getOwnerWindow()->popAppState();
}