// Derived From example 11-1 of "Learning OpenCV: Computer Vision with the OpenCV Library" by Gary Bradski

//-- includes -----
#include "CameraSettings/AppStage_CameraSettings.h"
#include "ModelFastenerCalibration/AppStage_ModelFastenerCalibration.h"
#include "ModelFastenerCalibration/RmlModel_ModelFastenerCalibration.h"
#include "App.h"
#include "Colors.h"
#include "GlCamera.h"
#include "GlLineRenderer.h"
#include "GlModelResourceManager.h"
#include "GlTextRenderer.h"
#include "GlFrameCompositor.h"
#include "GlRenderModelResource.h"
#include "GlWireframeMesh.h"
#include "InputManager.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "FastenerCalibrator.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "TextStyle.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VideoFrameDistortionView.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include "SDL_keycode.h"

#include "glm/gtc/quaternion.hpp"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

//-- statics ----
const char* AppStage_ModelFastenerCalibration::APP_STAGE_NAME = "ModelFastenerCalibration";

//-- public methods -----
AppStage_ModelFastenerCalibration::AppStage_ModelFastenerCalibration(App* app)
	: AppStage(app, AppStage_ModelFastenerCalibration::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_ModelFastenerCalibration)
	, m_targetFastenerId(INVALID_MIKAN_ID)
	, m_camera(nullptr)
	, m_modelResource(nullptr)
{
}

AppStage_ModelFastenerCalibration::~AppStage_ModelFastenerCalibration()
{
	delete m_calibrationModel;
}

void AppStage_ModelFastenerCalibration::enter()
{
	AppStage::enter();

	App* app= App::getInstance();
	const ProfileConfig* profileConfig = app->getProfileConfig();
	std::unique_ptr<class GlModelResourceManager>& modelResourceManager = 
		app->getRenderer()->getModelResourceManager();

	m_cameraTrackingPuckView =
		VRDeviceListIterator(eDeviceType::VRTracker, profileConfig->cameraVRDevicePath).getCurrent();

	// Create a new camera to view the scene
	m_camera = App::getInstance()->getRenderer()->pushCamera();
	m_camera->bindInput();

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		m_calibrationModel->init(context);
		m_calibrationModel->OnOkEvent = MakeDelegate(this, &AppStage_ModelFastenerCalibration::onOkEvent);
		m_calibrationModel->OnRedoEvent = MakeDelegate(this, &AppStage_ModelFastenerCalibration::onRedoEvent);
		m_calibrationModel->OnCancelEvent = MakeDelegate(this, &AppStage_ModelFastenerCalibration::onCancelEvent);

		// Init calibration view now that the dependent model has been created
		m_calibrationView = addRmlDocument("model_fastener_calibration.rml");
	}

	// Bind to space bar to capture frames
	// (Auto cleared on AppStage exit)
	{
		EventBindingSet* bindingSet = InputManager::getInstance()->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonReleasedEvent += MakeDelegate(this, &AppStage_ModelFastenerCalibration::onMouseButtonUp);
	}

	// Cache the stencil geometry
	MikanSpatialFastenerInfo fastenerInfo;
	if (profileConfig->getSpatialFastenerInfo(m_targetFastenerId, fastenerInfo) &&
		fastenerInfo.parent_object_type == MikanFastenerParentType_Stencil)
	{
		const MikanStencilModelConfig* stencil = profileConfig->getModelStencilConfig(fastenerInfo.parent_object_id);
		if (stencil != nullptr)
		{
			m_modelResource= modelResourceManager->fetchRenderModel(
				stencil->modelPath, 
				GlFrameCompositor::getStencilModelVertexDefinition());
		}
	}

	setMenuState(eModelFastenerCalibrationMenuState::verifyModel);
}

void AppStage_ModelFastenerCalibration::exit()
{
	setMenuState(eModelFastenerCalibrationMenuState::inactive);

	// Re-Enable depth testing on the line renderer while in this app stage
	Renderer::getInstance()->getLineRenderer()->setDisable3dDepth(false);

	App::getInstance()->getRenderer()->popCamera();
	m_camera= nullptr;

	AppStage::exit();
}

void AppStage_ModelFastenerCalibration::updateCamera()
{
	m_camera->recomputeModelViewMatrix();
}

void AppStage_ModelFastenerCalibration::update()
{
	AppStage::update();

	updateCamera();

	const eModelFastenerCalibrationMenuState currentMenuState= m_calibrationModel->getMenuState();
	eModelFastenerCalibrationMenuState nextMenuState= currentMenuState;

	switch(m_calibrationModel->getMenuState())
	{
		case eModelFastenerCalibrationMenuState::captureModelVertices:
			{
			}
			break;
		case eModelFastenerCalibrationMenuState::verifyVerticesCapture:
			{
			}
			break;
	}

	if (nextMenuState != currentMenuState)
	{
		setMenuState(nextMenuState);
	}
}

void AppStage_ModelFastenerCalibration::render()
{
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	renderModelScene();

	switch (m_calibrationModel->getMenuState())
	{
		case eModelFastenerCalibrationMenuState::verifyModel:
			{
			}
			break;
		case eModelFastenerCalibrationMenuState::captureModelVertices:
			{
			}
			break;
		case eModelFastenerCalibrationMenuState::verifyVerticesCapture:
			{
			}
			break;
	}
}

void AppStage_ModelFastenerCalibration::renderModelScene()
{
	if (m_modelResource != nullptr)
	{
		for (int meshIndex = 0; meshIndex < m_modelResource->getWireframeMeshCount(); ++meshIndex)
		{
			const GlWireframeMesh* mesh = m_modelResource->getWireframeMesh(meshIndex);

			if (mesh != nullptr)
			{
				drawTransformedWireframeMesh(glm::mat4(1.f), mesh, Colors::Yellow);
			}
		}
	}

	drawTransformedAxes(glm::mat4(1.f), 1.0f);

	TextStyle style = getDefaultTextStyle();
	drawTextAtWorldPosition(style, glm::vec3(1.f, 0.f, 0.f), L"X");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 1.f, 0.f), L"Y");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 1.f), L"Z");
}

void AppStage_ModelFastenerCalibration::setMenuState(eModelFastenerCalibrationMenuState newState)
{
	if (m_calibrationModel->getMenuState() != newState)
	{
		// Update menu state on the data models
		m_calibrationModel->setMenuState(newState);
	}
}

// Input Events
void AppStage_ModelFastenerCalibration::onMouseButtonUp(int button)
{
	if (m_calibrationModel->getMenuState() == eModelFastenerCalibrationMenuState::captureModelVertices)
	{
		if (button == SDL_BUTTON_LEFT)
		{
			int mouseScreenX = 0, mouseScreenY;
			InputManager::getInstance()->getMouseScreenPosition(mouseScreenX, mouseScreenY);

			Renderer* renderer = Renderer::getInstance();
			const float screenWidth = renderer->getSDLWindowWidth();
			const float screenHeight = renderer->getSDLWindowHeight();

			glm::vec2 pointSample((float)mouseScreenX,(float)mouseScreenY);
		}
	}
}

// Calibration Model UI Events
void AppStage_ModelFastenerCalibration::onOkEvent()
{
	switch (m_calibrationModel->getMenuState())
	{
		case eModelFastenerCalibrationMenuState::verifyModel:
			{
				setMenuState(eModelFastenerCalibrationMenuState::captureModelVertices);
			} break;
		case eModelFastenerCalibrationMenuState::captureModelVertices:
			{
				setMenuState(eModelFastenerCalibrationMenuState::verifyVerticesCapture);
			} break;
		case eModelFastenerCalibrationMenuState::verifyVerticesCapture:
			{
				m_app->popAppState();
			} break;
	}
}

void AppStage_ModelFastenerCalibration::onRedoEvent()
{
	setMenuState(eModelFastenerCalibrationMenuState::verifyModel);
}

void AppStage_ModelFastenerCalibration::onCancelEvent()
{
	m_app->popAppState();
}