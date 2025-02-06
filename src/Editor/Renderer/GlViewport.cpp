#include "App.h"
#include "EditorObjectSystem.h"
#include "GlViewport.h"
#include "SdlCommon.h"
#include "GlCamera.h"
#include "GlScene.h"
#include "GlStateStack.h"
#include "GlStateModifiers.h"
#include "MathUtility.h"
#include "Colors.h"
#include "InputManager.h"

#if defined(_WIN32)
#include <SDL_events.h>
#else
#include <SDL2/SDL_events.h>
#endif

// -- GlViewport --
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

	// Net valid until applyViewport
	m_renderOrigin= glm::i32vec2();
	m_renderSize= glm::i32vec2();
}

void GlViewport::setBackgroundColor(const glm::vec3& color)
{
	m_backgroundColor= glm::vec4(color, 1.f);
}

GlViewport::~GlViewport()
{
	unbindInput();
}

void GlViewport::applyRenderingViewport(GlState& glState) const
{
	glStateSetClearColor(glState, m_backgroundColor);

	// This calls onRenderingViewportApply from GLStateSetViewportImpl
	// onRenderingViewportRevert is called when the state is popped
	glStateSetViewport(
		glState, 
		m_viewportOrigin.x, m_windowSize.y - (m_viewportOrigin.y + m_viewportSize.y), 
		m_viewportSize.x, m_viewportSize.y);
}

void GlViewport::onRenderingViewportApply(int x, int y, int width, int height)
{
	m_renderOrigin = glm::i32vec2(x, y);
	m_renderSize = glm::i32vec2(width, height);
}

void GlViewport::onRenderingViewportRevert(int x, int y, int width, int height)
{
	m_renderOrigin = glm::i32vec2(x, y);
	m_renderSize = glm::i32vec2(width, height);
}

bool GlViewport::getRenderingViewport(glm::i32vec2& outOrigin, glm::i32vec2& outSize) const
{
	if (m_renderSize.x > 0 && m_renderSize.y > 0)
	{
		outOrigin = m_renderOrigin;
		outSize = m_renderSize;
		return true;
	}

	return false;
}

void GlViewport::update(float deltaSeconds)
{
	// Don't process input if the cursor isn't in the viewport
	glm::vec2 viewportLocation;
	if (!getCursorViewportPixelPos(viewportLocation))
		return;

	GlCameraPtr camera= getCurrentCamera();
	if (!camera)
		return;

	if (camera->getCameraMovementMode() == fly)
	{
		const float cameraSpeed = EditorObjectSystem::getSystem()->getEditorSystemConfig()->cameraSpeed;
		const float moveDelta = cameraSpeed * deltaSeconds;

		if (m_isLeftPressed || m_isRightPressed)
		{
			const float leftDelta = (m_isLeftPressed) ? -moveDelta : 0.f;
			const float rightDelta = (m_isRightPressed) ? moveDelta : 0.f;

			camera->adjustFlyRight(leftDelta + rightDelta);
		}

		if (m_isForwardPressed || m_isBackwardPressed)
		{
			const float forwardDelta = (m_isForwardPressed) ? moveDelta : 0.f;
			const float backwardDelta = (m_isBackwardPressed) ? -moveDelta : 0.f;

			camera->adjustFlyForward(forwardDelta + backwardDelta);
		}

		if (m_isUpPressed || m_isDownPressed)
		{
			const float upDelta = (m_isUpPressed) ? moveDelta : 0.f;
			const float downDelta = (m_isDownPressed) ? -moveDelta : 0.f;

			camera->adjustFlyUp(upDelta + downDelta);
		}
	}
}

GlCameraPtr GlViewport::getCurrentCamera() const
{
	return m_cameraPool[m_currentCameraIndex];
}

int GlViewport::getCurrentCameraIndex() const
{
	return m_currentCameraIndex;
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

GlCameraPtr GlViewport::getCameraByIndex(int cameraIndex)
{
	if (cameraIndex >= 0 && cameraIndex < getCameraCount())
	{
		return m_cameraPool[cameraIndex];
	}

	return nullptr;
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
		InputManager* inputManager= InputManager::getInstance();
		EventBindingSet* bindingSet = inputManager->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonPressedEvent += MakeDelegate(this, &GlViewport::onMouseButtonPressed);
		bindingSet->OnMouseButtonReleasedEvent += MakeDelegate(this, &GlViewport::onMouseButtonReleased);
		bindingSet->OnMouseMotionEvent += MakeDelegate(this, &GlViewport::onMouseMotion);
		bindingSet->OnMouseWheelScrolledEvent += MakeDelegate(this, &GlViewport::onMouseWheel);

		inputManager->fetchOrAddKeyBindings(SDLK_a)->OnKeyPressed += 
			MakeDelegate(this, &GlViewport::onLeftButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_a)->OnKeyReleased += 
			MakeDelegate(this, &GlViewport::onLeftButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_d)->OnKeyPressed += 
			MakeDelegate(this, &GlViewport::onRightButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_d)->OnKeyReleased += 
			MakeDelegate(this, &GlViewport::onRightButtonReleased);

		inputManager->fetchOrAddKeyBindings(SDLK_w)->OnKeyPressed +=
			MakeDelegate(this, &GlViewport::onForwardButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_w)->OnKeyReleased +=
			MakeDelegate(this, &GlViewport::onForwardButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_s)->OnKeyPressed +=
			MakeDelegate(this, &GlViewport::onBackwardButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_s)->OnKeyReleased +=
			MakeDelegate(this, &GlViewport::onBackwardButtonReleased);

		inputManager->fetchOrAddKeyBindings(SDLK_e)->OnKeyPressed +=
			MakeDelegate(this, &GlViewport::onUpButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_e)->OnKeyReleased +=
			MakeDelegate(this, &GlViewport::onUpButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_q)->OnKeyPressed +=
			MakeDelegate(this, &GlViewport::onDownButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_q)->OnKeyReleased +=
			MakeDelegate(this, &GlViewport::onDownButtonReleased);

		m_bIsInputBound = true;
	}
}

void GlViewport::unbindInput()
{
	if (m_bIsInputBound)
	{
		InputManager* inputManager = InputManager::getInstance();
		EventBindingSet* bindingSet = inputManager->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonPressedEvent -= MakeDelegate(this, &GlViewport::onMouseButtonPressed);
		bindingSet->OnMouseButtonReleasedEvent -= MakeDelegate(this, &GlViewport::onMouseButtonReleased);
		bindingSet->OnMouseMotionEvent -= MakeDelegate(this, &GlViewport::onMouseMotion);
		bindingSet->OnMouseWheelScrolledEvent -= MakeDelegate(this, &GlViewport::onMouseWheel);

		inputManager->fetchOrAddKeyBindings(SDLK_a)->OnKeyPressed -=
			MakeDelegate(this, &GlViewport::onLeftButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_a)->OnKeyReleased -=
			MakeDelegate(this, &GlViewport::onLeftButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_d)->OnKeyPressed -=
			MakeDelegate(this, &GlViewport::onRightButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_d)->OnKeyReleased -=
			MakeDelegate(this, &GlViewport::onRightButtonReleased);

		inputManager->fetchOrAddKeyBindings(SDLK_w)->OnKeyPressed -=
			MakeDelegate(this, &GlViewport::onForwardButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_w)->OnKeyReleased -=
			MakeDelegate(this, &GlViewport::onForwardButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_s)->OnKeyPressed -=
			MakeDelegate(this, &GlViewport::onBackwardButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_s)->OnKeyReleased -=
			MakeDelegate(this, &GlViewport::onBackwardButtonReleased);

		inputManager->fetchOrAddKeyBindings(SDLK_e)->OnKeyPressed -=
			MakeDelegate(this, &GlViewport::onUpButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_e)->OnKeyReleased -=
			MakeDelegate(this, &GlViewport::onUpButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_q)->OnKeyPressed -=
			MakeDelegate(this, &GlViewport::onDownButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_q)->OnKeyReleased -=
			MakeDelegate(this, &GlViewport::onDownButtonReleased);

		m_bIsInputBound = false;
	}
}

bool GlViewport::getCursorViewportPixelPos(glm::vec2& outViewportLocation) const
{
	int mouse_x, mouse_y;
	InputManager::getInstance()->getMouseScreenPosition(mouse_x, mouse_y);

	const int min_x = m_viewportOrigin.x;
	const int min_y = m_viewportOrigin.y;
	const int max_x = min_x + m_viewportSize.x;
	const int max_y = min_y + m_viewportSize.y;

	if (mouse_x >= min_x && mouse_x <= max_x && mouse_y >= min_y && mouse_y <= max_y)
	{
		outViewportLocation.x= (float)mouse_x - (float)min_x;
		outViewportLocation.y= (float)mouse_y - (float)min_y;
		return true;
	}

	return false;
}

void GlViewport::onMouseMotion(int deltaX, int deltaY)
{
	GlCameraPtr camera = getCurrentCamera();

	glm::vec2 viewportPos;
	if (camera && getCursorViewportPixelPos(viewportPos))
	{
		if (!m_isMouseInViewport)
		{
			m_isMouseInViewport = true;
			if (OnMouseEntered)
				OnMouseEntered();
		}

		glm::vec3 rayOrigin, rayDir;
		camera->computeCameraRayThruPixel(shared_from_this(), viewportPos, rayOrigin, rayDir);

		// Broadcast to any viewport raycast listeners
		if (OnMouseRayChanged)
			OnMouseRayChanged(rayOrigin, rayDir);

		if (m_isCameraRotateButtonPressed)
		{
			float deltaYaw = -(float)deltaX * k_camera_mouse_pan_scalar;
			float deltaPitch = (float)deltaY * k_camera_mouse_pan_scalar;

			switch (camera->getCameraMovementMode())
			{
				case eCameraMovementMode::fly:
					{
						if (!is_nearly_zero(deltaYaw))
							camera->adjustFlyYaw(deltaYaw);

						if (!is_nearly_zero(deltaPitch))
							camera->adjustFlyPitch(deltaPitch);
					} break;
				case eCameraMovementMode::orbit:
					{
						camera->adjustOrbitAngles(deltaYaw, deltaPitch);
					} break;
				default:
					break;
			}
		}
	}
	else
	{
		if (m_isMouseInViewport)
		{
			m_isMouseInViewport = false;
			if (OnMouseExited)
				OnMouseExited();
		}
	}
}

void GlViewport::onMouseButtonPressed(int button)
{
	GlCameraPtr camera = getCurrentCamera();

	glm::vec2 viewportPos;
	if (camera && getCursorViewportPixelPos(viewportPos))
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

void GlViewport::onMouseButtonReleased(int button)
{
	GlCameraPtr camera = getCurrentCamera();

	glm::vec2 viewportPos;
	if (camera && getCursorViewportPixelPos(viewportPos))
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
		camera->adjustOrbitRadius(deltaRadius);
	}
}