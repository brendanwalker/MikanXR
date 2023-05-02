#include "App.h"
#include "GlViewport.h"
#include "GlCommon.h"
#include "GlCamera.h"
#include "GlScene.h"
#include "Renderer.h"
#include "Colors.h"
#include "InputManager.h"

#if defined(_WIN32)
#include <SDL_events.h>
#else
#include <SDL2/SDL_events.h>
#endif

GlViewport::GlViewport()
	: m_backgroundColor(Colors::CornflowerBlue, 1.f)
{
	Renderer* renderer = App::getInstance()->getRenderer();

	m_windowSize= glm::i32vec2(
		(int)renderer->getSDLWindowWidth(),
		(int)renderer->getSDLWindowHeight());
	setViewport(glm::i32vec2(0, 0), m_windowSize);
	addCamera();
}

GlViewport::GlViewport(const glm::i32vec2& windowSize)
	: m_windowSize(windowSize)
	, m_backgroundColor(Colors::CornflowerBlue, 1.f)
{
	setViewport(glm::i32vec2(0, 0), m_windowSize);
	addCamera();
}

void GlViewport::setViewport(const glm::i32vec2& viewportOrigin, const glm::i32vec2& viewportSize)
{
	m_viewportOrigin = glm::max(glm::min(viewportOrigin, m_windowSize), glm::i32vec2(0, 0));
	m_viewportSize = glm::min((m_viewportOrigin + viewportSize), m_windowSize) - m_viewportOrigin;
}

void GlViewport::setBackgroundColor(const glm::vec3& color)
{
	m_backgroundColor= glm::vec4(color, 1.f);
}

GlViewport::~GlViewport()
{
	unbindInput();
}

void GlViewport::applyViewport() const
{
	glClearColor(m_backgroundColor.r, m_backgroundColor.g, m_backgroundColor.b, m_backgroundColor.a);
	glViewport(
		m_viewportOrigin.x, m_windowSize.y - (m_viewportOrigin.y + m_viewportSize.y), 
		m_viewportSize.x, m_viewportSize.y);
}

GlCameraPtr GlViewport::getCurrentCamera() const
{
	return m_cameraPool[m_currentCameraIndex];
}

GlCameraPtr GlViewport::addCamera()
{
	GlCameraPtr newCamera = std::make_shared<GlCamera>();
	m_cameraPool.push_back(newCamera);

	return newCamera;
}

int GlViewport::getCameraCount() const
{
	return (int)m_cameraPool.size();
}

void GlViewport::setCurrentCamera(int cameraIndex)
{
	if (cameraIndex >= 0 && cameraIndex < getCameraCount())
	{
		m_currentCameraIndex= cameraIndex;
	}
}

void GlViewport::bindInput()
{
	if (!m_bIsInputBound)
	{
		EventBindingSet* bindingSet = InputManager::getInstance()->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonPressedEvent += MakeDelegate(this, &GlViewport::onMouseButtonDown);
		bindingSet->OnMouseButtonReleasedEvent += MakeDelegate(this, &GlViewport::onMouseButtonUp);
		bindingSet->OnMouseMotionEvent += MakeDelegate(this, &GlViewport::onMouseMotion);
		bindingSet->OnMouseWheelScrolledEvent += MakeDelegate(this, &GlViewport::onMouseWheel);

		m_bIsInputBound = true;
	}
}

void GlViewport::unbindInput()
{
	if (m_bIsInputBound)
	{
		EventBindingSet* bindingSet = InputManager::getInstance()->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonPressedEvent -= MakeDelegate(this, &GlViewport::onMouseButtonDown);
		bindingSet->OnMouseButtonReleasedEvent -= MakeDelegate(this, &GlViewport::onMouseButtonUp);
		bindingSet->OnMouseMotionEvent -= MakeDelegate(this, &GlViewport::onMouseMotion);
		bindingSet->OnMouseWheelScrolledEvent -= MakeDelegate(this, &GlViewport::onMouseWheel);

		m_bIsInputBound = false;
	}
}

bool GlViewport::getCursorViewportLocation(glm::vec2& outViewportLocation) const
{
	int mouse_x, mouse_y;
	InputManager::getInstance()->getMouseScreenPosition(mouse_x, mouse_y);

	const int min_x = m_viewportOrigin.x;
	const int min_y = m_viewportOrigin.y;
	const int max_x = min_x + m_viewportSize.x;
	const int max_y = min_y + m_viewportSize.y;

	if (mouse_x >= min_x && mouse_x <= max_x && mouse_y >= min_y && mouse_y <= max_y)
	{
		outViewportLocation.x= ((float)mouse_x - (float)min_x) / (float)(max_x - min_x);
		outViewportLocation.y= ((float)mouse_y - (float)min_y) / (float)(max_y - min_y);
		return true;
	}

	return false;
}

void GlViewport::onMouseMotion(int deltaX, int deltaY)
{
	GlCameraPtr camera = getCurrentCamera();

	glm::vec2 viewportPos;
	if (camera && getCursorViewportLocation(viewportPos))
	{
		glm::vec3 rayOrigin, rayDir;
		camera->computeCameraRayThruPixel(shared_from_this(), viewportPos, rayOrigin, rayDir);

		// Broadcast to any viewport raycast listeners
		if (OnMouseRayChanged)
			OnMouseRayChanged(rayOrigin, rayDir);

		if (m_isCameraRotateButtonPressed)
		{
			float deltaYaw = -(float)deltaX * k_camera_mouse_pan_scalar;
			float deltaPitch = (float)deltaY * k_camera_mouse_pan_scalar;

			camera->adjustCameraOrbitAngles(deltaYaw, deltaPitch);
		}
	}
}

void GlViewport::onMouseButtonDown(int button)
{
	GlCameraPtr camera = getCurrentCamera();

	glm::vec2 viewportPos;
	if (camera && getCursorViewportLocation(viewportPos))
	{
		glm::vec3 rayOrigin, rayDir;
		camera->computeCameraRayThruPixel(shared_from_this(), viewportPos, rayOrigin, rayOrigin);

		// Broadcast to any viewport raycast listeners
		if (OnMouseRayButtonDown)
			OnMouseRayButtonDown(rayOrigin, rayDir, button);

		if (button == SDL_BUTTON_RIGHT)
		{
			m_isCameraRotateButtonPressed = true;
		}
	}
}

void GlViewport::onMouseButtonUp(int button)
{
	GlCameraPtr camera = getCurrentCamera();

	glm::vec2 viewportPos;
	if (camera && getCursorViewportLocation(viewportPos))
	{
		glm::vec3 rayOrigin, rayDir;
		camera->computeCameraRayThruPixel(shared_from_this(), viewportPos, rayOrigin, rayOrigin);

		// Broadcast to any viewport raycast listeners
		if (OnMouseRayButtonUp)
			OnMouseRayButtonUp(rayOrigin, rayDir, button);

		if (button == SDL_BUTTON_RIGHT)
		{
			m_isCameraRotateButtonPressed = false;
		}
	}
}

void GlViewport::onMouseWheel(int scrollAmount)
{
	float deltaRadius = (float)scrollAmount * k_camera_mouse_zoom_scalar;

	GlCameraPtr camera = getCurrentCamera();
	if (camera)
	{
		camera->adjustCameraOrbitRadius(deltaRadius);
	}
}