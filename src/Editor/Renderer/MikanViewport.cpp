#include "App.h"
#include "EditorObjectSystem.h"
#include "MikanViewport.h"
#include "SdlCommon.h"
#include "MikanCamera.h"
#include "GlScene.h"
#include "MkStateStack.h"
#include "MkStateModifiers.h"
#include "MathUtility.h"
#include "Colors.h"
#include "InputManager.h"

#if defined(_WIN32)
#include <SDL_events.h>
#else
#include <SDL2/SDL_events.h>
#endif

// -- GlViewport --
MikanViewport::MikanViewport(const glm::i32vec2& windowSize)
	: m_windowSize(windowSize)
	, m_backgroundColor(Colors::CornflowerBlue, 1.f)
{
	setViewport(glm::i32vec2(0, 0), m_windowSize);
	addCamera();
}

void MikanViewport::setViewport(const glm::i32vec2& viewportOrigin, const glm::i32vec2& viewportSize)
{
	m_viewportOrigin = glm::max(glm::min(viewportOrigin, m_windowSize), glm::i32vec2(0, 0));
	m_viewportSize = glm::min((m_viewportOrigin + viewportSize), m_windowSize) - m_viewportOrigin;

	// Net valid until applyViewport
	m_renderOrigin= glm::i32vec2();
	m_renderSize= glm::i32vec2();
}

void MikanViewport::setBackgroundColor(const glm::vec3& color)
{
	m_backgroundColor= glm::vec4(color, 1.f);
}

MikanViewport::~MikanViewport()
{
	unbindInput();
}

void MikanViewport::applyRenderingViewport(IMkState* glState) const
{
	mkStateSetClearColor(glState, m_backgroundColor);

	// This calls onRenderingViewportApply from mkStateSetViewportImpl
	// onRenderingViewportRevert is called when the state is popped
	mkStateSetViewport(
		glState, 
		m_viewportOrigin.x, m_windowSize.y - (m_viewportOrigin.y + m_viewportSize.y), 
		m_viewportSize.x, m_viewportSize.y);
}

void MikanViewport::onRenderingViewportApply(int x, int y, int width, int height)
{
	m_renderOrigin = glm::i32vec2(x, y);
	m_renderSize = glm::i32vec2(width, height);
}

void MikanViewport::onRenderingViewportRevert(int x, int y, int width, int height)
{
	m_renderOrigin = glm::i32vec2(x, y);
	m_renderSize = glm::i32vec2(width, height);
}

bool MikanViewport::getRenderingViewport(glm::i32vec2& outOrigin, glm::i32vec2& outSize) const
{
	if (m_renderSize.x > 0 && m_renderSize.y > 0)
	{
		outOrigin = m_renderOrigin;
		outSize = m_renderSize;
		return true;
	}

	return false;
}

void MikanViewport::update(float deltaSeconds)
{
	// Don't process input if the cursor isn't in the viewport
	glm::vec2 viewportLocation;
	if (!getCursorViewportPixelPos(viewportLocation))
		return;

	MikanCameraPtr camera= std::static_pointer_cast<MikanCamera>(getCurrentCamera());
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

IMkCameraPtr MikanViewport::getCurrentCamera() const
{
	return m_cameraPool[m_currentCameraIndex];
}

int MikanViewport::getCurrentCameraIndex() const
{
	return m_currentCameraIndex;
}

IMkCameraPtr MikanViewport::addCamera()
{
	MikanCameraPtr newCamera = std::make_shared<MikanCamera>();
	m_cameraPool.push_back(newCamera);

	return newCamera;
}

int MikanViewport::getCameraCount() const
{
	return (int)m_cameraPool.size();
}

IMkCameraPtr MikanViewport::getCameraByIndex(int cameraIndex)
{
	if (cameraIndex >= 0 && cameraIndex < getCameraCount())
	{
		return m_cameraPool[cameraIndex];
	}

	return nullptr;
}

void MikanViewport::setCurrentCamera(int cameraIndex)
{
	if (cameraIndex >= 0 && cameraIndex < getCameraCount())
	{
		m_currentCameraIndex= cameraIndex;
	}
}

MikanCameraPtr MikanViewport::getCurrentMikanCamera() const
{
	return std::static_pointer_cast<MikanCamera>(getCurrentCamera());
}

MikanCameraPtr MikanViewport::addMikanCamera()
{
	return std::static_pointer_cast<MikanCamera>(addCamera());
}

MikanCameraPtr MikanViewport::getMikanCameraByIndex(int cameraIndex)
{
	return std::static_pointer_cast<MikanCamera>(getCameraByIndex(cameraIndex));
}

void MikanViewport::bindInput()
{
	if (!m_bIsInputBound)
	{
		InputManager* inputManager= InputManager::getInstance();
		EventBindingSet* bindingSet = inputManager->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonPressedEvent += MakeDelegate(this, &MikanViewport::onMouseButtonPressed);
		bindingSet->OnMouseButtonReleasedEvent += MakeDelegate(this, &MikanViewport::onMouseButtonReleased);
		bindingSet->OnMouseMotionEvent += MakeDelegate(this, &MikanViewport::onMouseMotion);
		bindingSet->OnMouseWheelScrolledEvent += MakeDelegate(this, &MikanViewport::onMouseWheel);

		inputManager->fetchOrAddKeyBindings(SDLK_a)->OnKeyPressed += 
			MakeDelegate(this, &MikanViewport::onLeftButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_a)->OnKeyReleased += 
			MakeDelegate(this, &MikanViewport::onLeftButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_d)->OnKeyPressed += 
			MakeDelegate(this, &MikanViewport::onRightButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_d)->OnKeyReleased += 
			MakeDelegate(this, &MikanViewport::onRightButtonReleased);

		inputManager->fetchOrAddKeyBindings(SDLK_w)->OnKeyPressed +=
			MakeDelegate(this, &MikanViewport::onForwardButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_w)->OnKeyReleased +=
			MakeDelegate(this, &MikanViewport::onForwardButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_s)->OnKeyPressed +=
			MakeDelegate(this, &MikanViewport::onBackwardButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_s)->OnKeyReleased +=
			MakeDelegate(this, &MikanViewport::onBackwardButtonReleased);

		inputManager->fetchOrAddKeyBindings(SDLK_e)->OnKeyPressed +=
			MakeDelegate(this, &MikanViewport::onUpButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_e)->OnKeyReleased +=
			MakeDelegate(this, &MikanViewport::onUpButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_q)->OnKeyPressed +=
			MakeDelegate(this, &MikanViewport::onDownButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_q)->OnKeyReleased +=
			MakeDelegate(this, &MikanViewport::onDownButtonReleased);

		m_bIsInputBound = true;
	}
}

void MikanViewport::unbindInput()
{
	if (m_bIsInputBound)
	{
		InputManager* inputManager = InputManager::getInstance();
		EventBindingSet* bindingSet = inputManager->getCurrentEventBindingSet();

		bindingSet->OnMouseButtonPressedEvent -= MakeDelegate(this, &MikanViewport::onMouseButtonPressed);
		bindingSet->OnMouseButtonReleasedEvent -= MakeDelegate(this, &MikanViewport::onMouseButtonReleased);
		bindingSet->OnMouseMotionEvent -= MakeDelegate(this, &MikanViewport::onMouseMotion);
		bindingSet->OnMouseWheelScrolledEvent -= MakeDelegate(this, &MikanViewport::onMouseWheel);

		inputManager->fetchOrAddKeyBindings(SDLK_a)->OnKeyPressed -=
			MakeDelegate(this, &MikanViewport::onLeftButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_a)->OnKeyReleased -=
			MakeDelegate(this, &MikanViewport::onLeftButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_d)->OnKeyPressed -=
			MakeDelegate(this, &MikanViewport::onRightButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_d)->OnKeyReleased -=
			MakeDelegate(this, &MikanViewport::onRightButtonReleased);

		inputManager->fetchOrAddKeyBindings(SDLK_w)->OnKeyPressed -=
			MakeDelegate(this, &MikanViewport::onForwardButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_w)->OnKeyReleased -=
			MakeDelegate(this, &MikanViewport::onForwardButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_s)->OnKeyPressed -=
			MakeDelegate(this, &MikanViewport::onBackwardButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_s)->OnKeyReleased -=
			MakeDelegate(this, &MikanViewport::onBackwardButtonReleased);

		inputManager->fetchOrAddKeyBindings(SDLK_e)->OnKeyPressed -=
			MakeDelegate(this, &MikanViewport::onUpButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_e)->OnKeyReleased -=
			MakeDelegate(this, &MikanViewport::onUpButtonReleased);
		inputManager->fetchOrAddKeyBindings(SDLK_q)->OnKeyPressed -=
			MakeDelegate(this, &MikanViewport::onDownButtonPressed);
		inputManager->fetchOrAddKeyBindings(SDLK_q)->OnKeyReleased -=
			MakeDelegate(this, &MikanViewport::onDownButtonReleased);

		m_bIsInputBound = false;
	}
}

bool MikanViewport::getCursorViewportPixelPos(glm::vec2& outViewportLocation) const
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

void MikanViewport::onMouseMotion(int deltaX, int deltaY)
{
	MikanCameraPtr camera = std::static_pointer_cast<MikanCamera>(getCurrentCamera());

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

void MikanViewport::onMouseButtonPressed(int button)
{
	MikanCameraPtr camera = std::static_pointer_cast<MikanCamera>(getCurrentCamera());

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

void MikanViewport::onMouseButtonReleased(int button)
{
	MikanCameraPtr camera = std::static_pointer_cast<MikanCamera>(getCurrentCamera());

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

void MikanViewport::onMouseWheel(int scrollAmount)
{
	float deltaRadius = (float)scrollAmount * k_camera_mouse_zoom_scalar;

	MikanCameraPtr camera = std::static_pointer_cast<MikanCamera>(getCurrentCamera());
	if (camera)
	{
		camera->adjustOrbitRadius(deltaRadius);
	}
}