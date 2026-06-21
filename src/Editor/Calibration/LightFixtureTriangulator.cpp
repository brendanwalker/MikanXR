#include "LightFixtureTriangulator.h"
#include "CalibrationRenderHelpers.h"
#include "CameraComponent.h"
#include "CameraMath.h"
#include "Colors.h"
#include "IEditorWindow.h"
#include "InputManager.h"
#include "MathGLM.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "MikanLineRenderer.h"
#include "MikanTextRenderer.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"

struct LightFixtureTriangulationState
{
	// Camera intrinsics (for ray math)
	MikanMonoIntrinsics inputCameraIntrinsics;

	// Camera 1 sample
	glm::vec2 initialPixelSample;
	glm::mat4 initialCameraPose;
	bool hasInitialSample;

	// Live triangulation result
	glm::vec3 lastTriangulatedPoint;

	// Confirmed triangulated position
	glm::vec3 triangulatedPosition;
	bool hasTriangulatedPosition;

	void init(CameraComponentPtr cameraComponent)
	{
		MikanVideoSourceIntrinsics cameraIntrinsics;
		cameraComponent->getApertureIntrinsics(cameraIntrinsics);
		inputCameraIntrinsics= cameraIntrinsics.getMonoIntrinsics();

		reset();
	}

	void reset()
	{
		initialPixelSample= glm::vec2();
		initialCameraPose= glm::mat4(1.f);
		hasInitialSample= false;
		lastTriangulatedPoint= glm::vec3();
		triangulatedPosition= glm::vec3();
		hasTriangulatedPosition= false;
	}
};

// -- LightFixtureTriangulator -----
LightFixtureTriangulator::LightFixtureTriangulator(CameraComponentPtr cameraComponent,
												   VideoFrameDistortionView* distortionView)
	: m_cameraComponent(cameraComponent)
	, m_distortionView(distortionView)
	, m_state(new LightFixtureTriangulationState)
{
	m_frameWidth= distortionView->getFrameWidth();
	m_frameHeight= distortionView->getFrameHeight();
	m_state->init(cameraComponent);
}

LightFixtureTriangulator::~LightFixtureTriangulator() { delete m_state; }

bool LightFixtureTriangulator::hasInitialSample() const { return m_state->hasInitialSample; }

bool LightFixtureTriangulator::hasTriangulatedPosition() const { return m_state->hasTriangulatedPosition; }

void LightFixtureTriangulator::resetCalibrationState() { m_state->reset(); }

void LightFixtureTriangulator::sampleCameraPose()
{
	glm::mat4 cameraPose;
	if (m_cameraComponent->getStageSpaceAperturePose(cameraPose))
	{
		m_state->initialCameraPose= cameraPose;
	}
}

void LightFixtureTriangulator::sampleMouseScreenPosition()
{
	if (!m_state->hasInitialSample)
	{
		m_state->initialPixelSample= computeMouseScreenPosition();
		m_state->hasInitialSample= true;
	}
	else if (!m_state->hasTriangulatedPosition)
	{
		m_state->triangulatedPosition= m_state->lastTriangulatedPoint;
		m_state->hasTriangulatedPosition= true;
	}
}

void LightFixtureTriangulator::computeCurrentTriangulation()
{
	if (!m_state->hasInitialSample)
		return;

	glm::vec3 ray1Start, ray1Dir;
	computeCameraRayAtPixel(m_state->inputCameraIntrinsics, m_state->initialCameraPose, m_state->initialPixelSample,
							ray1Start, ray1Dir);

	glm::mat4 currentCameraPose;
	if (m_cameraComponent->getStageSpaceAperturePose(currentCameraPose))
	{
		glm::vec3 ray2Start, ray2Dir;
		computeCameraRayAtPixel(m_state->inputCameraIntrinsics, currentCameraPose, computeMouseScreenPosition(),
								ray2Start, ray2Dir);

		float closestTime;
		glm::vec3 closestPoint;
		if (glm_closest_point_on_ray_to_ray(ray1Start, ray1Dir, ray2Start, ray2Dir, closestTime, closestPoint)
			&& closestTime >= 0.f)
		{
			m_state->lastTriangulatedPoint= closestPoint;
		}
	}
}

bool LightFixtureTriangulator::getTriangulatedPosition(glm::vec3& outPosition) const
{
	if (!m_state->hasTriangulatedPosition)
		return false;
	outPosition= m_state->triangulatedPosition;
	return true;
}

glm::vec2 LightFixtureTriangulator::computeMouseScreenPosition() const
{
	IEditorWindow* window= m_cameraComponent->getOwnerEditorWindow();

	int mouseScreenX= 0, mouseScreenY= 0;
	window->getInputManager()->getMouseScreenPosition(mouseScreenX, mouseScreenY);

	const float screenWidth= window->getWidth();
	const float screenHeight= window->getHeight();

	return glm::vec2(((float)mouseScreenX * m_frameWidth) / screenWidth,
					 ((float)mouseScreenY * m_frameHeight) / screenHeight);
}

void LightFixtureTriangulator::renderInitialPoint2d()
{
	if (!m_state->hasInitialSample)
		return;

	IMkGraphicsContext* graphicsContext= m_cameraComponent->getGraphicsContext();
	const glm::vec3 point(m_state->initialPixelSample.x, m_state->initialPixelSample.y, 0.5f);

	glm::vec3 points[1]= {point};
	drawPointList2d(graphicsContext, m_frameWidth, m_frameHeight, points, 1, Colors::Yellow, 2.f);

	TextStyle style= getDefaultTextStyle();
	style.horizontalAlignment= eHorizontalTextAlignment::Middle;
	style.verticalAlignment= eVerticalTextAlignment::Bottom;
	drawTextAtCameraPosition(graphicsContext, style, m_frameWidth, m_frameHeight, point, L"Light");
}

void LightFixtureTriangulator::renderCurrentTriangulation()
{
	if (!m_state->hasInitialSample)
		return;

	IMkGraphicsContext* graphicsContext= m_cameraComponent->getGraphicsContext();

	// Draw initial ray
	glm::vec3 rayStart, rayDir;
	computeCameraRayAtPixel(m_state->inputCameraIntrinsics, m_state->initialCameraPose, m_state->initialPixelSample,
							rayStart, rayDir);
	drawSegment(graphicsContext, glm::mat4(1.f), rayStart, rayStart + rayDir * 1000.f, Colors::Yellow);

	// Draw current live triangulated point
	drawPoint(graphicsContext, glm::mat4(1.f), m_state->lastTriangulatedPoint, Colors::Yellow, 5.f);
}

void LightFixtureTriangulator::renderTriangulatedPosition()
{
	if (!m_state->hasTriangulatedPosition)
		return;

	IMkGraphicsContext* graphicsContext= m_cameraComponent->getGraphicsContext();

	drawPoint(graphicsContext, glm::mat4(1.f), m_state->triangulatedPosition, Colors::Cyan, 8.f);

	TextStyle style= getDefaultTextStyle();
	drawTextAtWorldPosition(graphicsContext, style, m_state->triangulatedPosition, L"Light Position");
}
