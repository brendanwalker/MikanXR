//-- inludes -----
#include "App.h"
#include "ClientTextureSourceComponent.h"
#include "IMkTriangulatedMesh.h"
#include "TextureSourceSettings/AppStage_TextureSourceSettings.h"
#include "TextureSourceSettings/RmlModel_TextureSourceSettings.h"
#include "Shared/RmlModel_TextureSourceComponent.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "MikanTextRenderer.h"
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

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init Data Models
		auto* TextureSourceSettingsModel = addRmlModel<RmlModel_TextureSourceSettings>();
		TextureSourceSettingsModel->init(context);
		TextureSourceSettingsModel->OnReturnEvent = MakeDelegate(this, &AppStage_TextureSourceSettings::onReturnEvent);

		auto* clientTextureSourceComponentModel = addRmlModel<RmlModel_ClientTextureSourceComponent>();
		clientTextureSourceComponentModel->init(context);
		if (auto clientTextureSourceComponent =
			std::dynamic_pointer_cast<ClientTextureSourceComponent>(textureSourceComponent))
		{
			clientTextureSourceComponentModel->setComponent(clientTextureSourceComponent);
		}

		auto* spoutTextureSourceComponentModel = addRmlModel<RmlModel_SpoutTextureSourceComponent>();
		spoutTextureSourceComponentModel->init(context);
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

	// Create a mesh used to render the video frame
	m_fullscreenRGBQuad = createFullscreenQuadMesh(m_ownerWindow, true, false);
	m_fullscreenRGBAQuad = createFullscreenQuadMesh(m_ownerWindow, true, true);
}

void AppStage_TextureSourceSettings::render(IMkViewportPtr targetViewport)
{
	TextureSourceComponentPtr textureSourceComponent = m_textureSourceComponent.lock();

	IMkTriangulatedMeshPtr fullscreenQuad;
	IMkTexturePtr videoTexture = textureSourceComponent->getClientColorSourceTexture(eTextureSourceColorType::colorRGBA);
	eUniformSemantic videoTextureSemantic = eUniformSemantic::rgbaTexture;
	if (videoTexture)
	{
		fullscreenQuad = m_fullscreenRGBAQuad;
	}
	else
	{
		videoTexture = textureSourceComponent->getClientColorSourceTexture(eTextureSourceColorType::colorRGB);
		if (videoTexture)
		{
			fullscreenQuad = m_fullscreenRGBQuad;
			videoTextureSemantic = eUniformSemantic::rgbaTexture;
		}
	}

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

void AppStage_TextureSourceSettings::onReturnEvent()
{
	getOwnerWindow()->popAppState();
}