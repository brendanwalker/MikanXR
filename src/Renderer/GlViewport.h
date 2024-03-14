#pragma once

#include "MulticastDelegate.h"
#include "RendererFwd.h"

#include "memory"
#include "vector"

#include "glm/ext/vector_int2_sized.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

class GlViewport : public std::enable_shared_from_this<GlViewport>
{
public:
	GlViewport(const glm::i32vec2& windowSize);
	virtual ~GlViewport();

	glm::i32vec2 getViewportOrigin() const { return m_viewportOrigin; }
	glm::i32vec2 getViewportSize() const { return m_viewportSize; }

	void setViewport(const glm::i32vec2& viewportOrigin, const glm::i32vec2& viewportSize);
	void setBackgroundColor(const glm::vec3& color);

	void bindInput();
	void unbindInput();

	void applyViewport(GlState& glState) const;
	void update(float deltaSeconds);

	GlCameraPtr getCurrentCamera() const;
	int getCurrentCameraIndex() const;
	GlCameraPtr addCamera();
	int getCameraCount() const;
	GlCameraPtr getCameraByIndex(int cameraIndex);
	void setCurrentCamera(int cameraIndex);
	bool getIsMouseInViewport() const { return m_isMouseInViewport; }

	// Convert cursor pixel position from app window relative to viewport relative
	bool getCursorViewportPixelPos(glm::vec2& outViewportLocation) const;

	MulticastDelegate<void()> OnMouseEntered;
	MulticastDelegate<void()> OnMouseExited;
	MulticastDelegate<void(const glm::vec3& rayOrigin, const glm::vec3& rayDir)> OnMouseRayChanged;
	MulticastDelegate<void(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)> OnMouseRayButtonDown;
	MulticastDelegate<void(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)> OnMouseRayButtonUp;

protected:
	void onMouseMotion(int deltaX, int deltaY);
	void onMouseButtonPressed(int button);
	void onMouseButtonReleased(int button);
	void onMouseWheel(int scrollAmount);

	void onLeftButtonPressed() { m_isLeftPressed= true; }
	void onLeftButtonReleased() { m_isLeftPressed= false; }
	void onRightButtonPressed() { m_isRightPressed= true; }
	void onRightButtonReleased() { m_isRightPressed= false; }

	void onForwardButtonPressed() { m_isForwardPressed= true; }
	void onForwardButtonReleased() { m_isForwardPressed= false; }
	void onBackwardButtonPressed() { m_isBackwardPressed= true; }
	void onBackwardButtonReleased() { m_isBackwardPressed= false; }

	void onUpButtonPressed() { m_isUpPressed= true; }
	void onUpButtonReleased() { m_isUpPressed= false; }
	void onDownButtonPressed() { m_isDownPressed= true; }
	void onDownButtonReleased() { m_isDownPressed= false; }

private:
	bool m_bIsInputBound= false;
	bool m_isCameraRotateButtonPressed= false;
	bool m_isLeftPressed= false;
	bool m_isRightPressed= false;
	bool m_isForwardPressed= false;
	bool m_isBackwardPressed= false;
	bool m_isUpPressed= false;
	bool m_isDownPressed= false;
	bool m_isMouseInViewport= false;

	glm::i32vec2 m_windowSize;
	glm::i32vec2 m_viewportOrigin;
	glm::i32vec2 m_viewportSize;
	glm::vec4 m_backgroundColor;

	std::vector<GlCameraPtr> m_cameraPool;
	int m_currentCameraIndex= 0;

	const float k_camera_mouse_zoom_scalar = 0.1f;
	const float k_camera_mouse_pan_scalar = 0.25f;
	const float k_camera_min_zoom = 0.01f;
};
