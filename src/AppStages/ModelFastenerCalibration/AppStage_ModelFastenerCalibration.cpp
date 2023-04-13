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
#include "GlViewport.h"
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
static const glm::vec3 k_modelFastenerColors[3] = {Colors::Red, Colors::Green, Colors::Blue};
const char* AppStage_ModelFastenerCalibration::APP_STAGE_NAME = "ModelFastenerCalibration";

struct ModelFastenerCalibrationState
{
	glm::vec3 closestModelVertex;
	bool closestModelVertexValid = false;

	glm::vec3 capturedVertices[3];
	int capturedVertexCount = 0.f;

	void resetClosestModelVertex()
	{
		closestModelVertex = glm::vec3(0.f);
		closestModelVertexValid = false;
	}

	void setClosestModelVertex(const glm::vec3& vertex)
	{
		closestModelVertex = vertex;
		closestModelVertexValid = true;
	}

	void resetCapturedVertices()
	{
		capturedVertices[0] = glm::vec3(0.f);
		capturedVertices[1] = glm::vec3(0.f);
		capturedVertices[2] = glm::vec3(0.f);
		capturedVertexCount = 0;
	}

	void captureClosestModelVertex()
	{
		if (closestModelVertexValid && capturedVertexCount < 3)
		{
			capturedVertices[capturedVertexCount] = closestModelVertex;
			capturedVertexCount++;
		}
	}

	void reset()
	{
		resetClosestModelVertex();
		resetCapturedVertices();
	}
};

//-- public methods -----
AppStage_ModelFastenerCalibration::AppStage_ModelFastenerCalibration(App* app)
	: AppStage(app, AppStage_ModelFastenerCalibration::APP_STAGE_NAME)
	, m_calibrationModel(new RmlModel_ModelFastenerCalibration)
	, m_calibrationState(new ModelFastenerCalibrationState)
	, m_modelResource(nullptr)
{
	memset(&m_targetFastener, 0, sizeof(MikanSpatialFastenerInfo));
	m_targetFastener.parent_object_type = MikanFastenerParentType_UNKNOWN;

	m_calibrationState->reset();
}

AppStage_ModelFastenerCalibration::~AppStage_ModelFastenerCalibration()
{
	delete m_calibrationModel;
	delete m_calibrationState;
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
	if (m_targetFastener.parent_object_type == MikanFastenerParentType_Stencil)
	{
		const MikanStencilModelConfig* stencil = 
			profileConfig->getModelStencilConfig(m_targetFastener.parent_object_id);

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

	AppStage::exit();
}

void AppStage_ModelFastenerCalibration::updateCamera()
{
	getFirstViewport()->getCurrentCamera()->recomputeModelViewMatrix();
}

void AppStage_ModelFastenerCalibration::updateClosestModelVertex()
{
	int mouseScreenX = 0, mouseScreenY;
	InputManager::getInstance()->getMouseScreenPosition(mouseScreenX, mouseScreenY);
	const glm::vec2 pixelLocation((float)mouseScreenX, (float)mouseScreenY);

	glm::vec3 rayOrigin, rayDirection;
	GlViewportPtr viewport= getFirstViewport();
	GlCameraPtr camera= viewport->getCurrentCamera();
	camera->computeCameraRayThruPixel(viewport, pixelLocation, rayOrigin, rayDirection);

	float closestDotProduct= -1.f;

	m_calibrationState->resetClosestModelVertex();

	for (int meshIndex = 0; meshIndex < (int)m_modelResource->getWireframeMeshCount(); ++meshIndex)
	{
		GlWireframeMeshPtr wireframeMesh= m_modelResource->getWireframeMesh(meshIndex);

		const uint32_t vertexCount= wireframeMesh->getVertexCount();
		const glm::vec3* vertexData= (glm::vec3*)wireframeMesh->getVertexData();

		for (uint32_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
		{
			const glm::vec3& vertex= vertexData[vertexIndex];
			const glm::vec3 rayOriginToVertex= glm::normalize(vertex - rayOrigin);
			const float dotProduct= glm::dot(rayDirection, rayOriginToVertex);

			if (!m_calibrationState->closestModelVertexValid || 
				dotProduct > closestDotProduct)
			{
				closestDotProduct = dotProduct;
				m_calibrationState->setClosestModelVertex(vertex);
			}
		}
	}
}

void AppStage_ModelFastenerCalibration::update()
{
	AppStage::update();

	updateCamera();

	const eModelFastenerCalibrationMenuState currentMenuState= m_calibrationModel->getMenuState();
	eModelFastenerCalibrationMenuState nextMenuState= currentMenuState;

	if (m_calibrationModel->getMenuState() == eModelFastenerCalibrationMenuState::captureModelVertices)
	{
		updateClosestModelVertex();

		if (m_calibrationState->capturedVertexCount >= 3)
		{
			nextMenuState= eModelFastenerCalibrationMenuState::verifyVerticesCapture;
		}
	}

	if (nextMenuState != currentMenuState)
	{
		setMenuState(nextMenuState);
	}
}

void AppStage_ModelFastenerCalibration::render()
{
	switch (m_calibrationModel->getMenuState())
	{
		case eModelFastenerCalibrationMenuState::verifyModel:
			renderModelScene();
			break;
		case eModelFastenerCalibrationMenuState::captureModelVertices:
			renderModelScene();
			renderClosestModelVertex();
			renderCaptureModelVertices();
			break;
		case eModelFastenerCalibrationMenuState::verifyVerticesCapture:
			renderModelScene();
			renderCaptureModelVertices();
			break;
	}
}

void AppStage_ModelFastenerCalibration::renderModelScene() const
{
	if (m_modelResource != nullptr)
	{
		for (int meshIndex = 0; meshIndex < m_modelResource->getWireframeMeshCount(); ++meshIndex)
		{
			GlWireframeMeshPtr mesh = m_modelResource->getWireframeMesh(meshIndex);

			if (mesh != nullptr)
			{
				drawTransformedWireframeMesh(glm::mat4(1.f), mesh.get(), Colors::Gray);
			}
		}
	}

	drawTransformedAxes(glm::mat4(1.f), 1.0f);

	TextStyle style = getDefaultTextStyle();
	drawTextAtWorldPosition(style, glm::vec3(1.f, 0.f, 0.f), L"X");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 1.f, 0.f), L"Y");
	drawTextAtWorldPosition(style, glm::vec3(0.f, 0.f, 1.f), L"Z");
}

void AppStage_ModelFastenerCalibration::renderClosestModelVertex() const
{
	int mouseScreenX = 0, mouseScreenY;
	InputManager::getInstance()->getMouseScreenPosition(mouseScreenX, mouseScreenY);
	const glm::vec2 pixelLocation((float)mouseScreenX, (float)mouseScreenY);

	glm::vec3 rayOrigin, rayDirection;
	GlViewportPtr viewport = getFirstViewport();
	GlCameraPtr camera = viewport->getCurrentCamera();
	camera->computeCameraRayThruPixel(viewport, pixelLocation, rayOrigin, rayDirection);

	drawArrow(glm::mat4(1.f), rayOrigin, rayOrigin + rayDirection * 1.f, 0.01f, Colors::CornflowerBlue);

	// Draw the closest point
	if (m_calibrationState->closestModelVertexValid)
	{
		drawPoint(glm::mat4(1.f), m_calibrationState->closestModelVertex, Colors::Yellow, 5.f);
	}
}

void AppStage_ModelFastenerCalibration::renderCaptureModelVertices() const
{
	for (int i = 0; i < m_calibrationState->capturedVertexCount; i++)
	{
		const glm::vec3& vertex = m_calibrationState->capturedVertices[i];

		TextStyle style = getDefaultTextStyle();
		style.horizontalAlignment = eHorizontalTextAlignment::Middle;
		style.verticalAlignment = eVerticalTextAlignment::Bottom;
		drawTextAtWorldPosition(style, vertex, L"P%d", i);

		drawPoint(glm::mat4(1.f), vertex, k_modelFastenerColors[i], 5.f);

		if (i >= 1)
		{
			drawSegment(
				glm::mat4(1.f),
				m_calibrationState->capturedVertices[0], m_calibrationState->capturedVertices[i],
				k_modelFastenerColors[0], k_modelFastenerColors[i]);
		}
	}
}

void AppStage_ModelFastenerCalibration::setMenuState(eModelFastenerCalibrationMenuState newState)
{
	if (m_calibrationModel->getMenuState() != newState)
	{
		// Handle state entry
		switch (newState)
		{
			case eModelFastenerCalibrationMenuState::verifyModel:
			{

			} break;
			case eModelFastenerCalibrationMenuState::captureModelVertices:
			{
				m_calibrationState->reset();
			} break;
			case eModelFastenerCalibrationMenuState::verifyVerticesCapture:
			{

			} break;
		}

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
			m_calibrationState->captureClosestModelVertex();
			m_calibrationModel->setCapturedPointCount(m_calibrationState->capturedVertexCount);
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
		case eModelFastenerCalibrationMenuState::verifyVerticesCapture:
			{
				ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

				for (int i = 0; i < 3; i++)
				{
					m_targetFastener.fastener_points[i]= 
						glm_vec3_to_MikanVector3f(m_calibrationState->capturedVertices[i]);
				}

				if (m_targetFastener.fastener_id == INVALID_MIKAN_ID)
				{
					profileConfig->addNewFastener(m_targetFastener);
				}
				else
				{
					profileConfig->updateFastener(m_targetFastener);
				}

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