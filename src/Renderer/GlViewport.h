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
	GlViewport();
	GlViewport(const glm::i32vec2& windowSize);
	virtual ~GlViewport();

	glm::i32vec2 getViewportOrigin() const { return m_viewportOrigin; }
	glm::i32vec2 getViewportSize() const { return m_viewportSize; }

	void setViewport(const glm::i32vec2& viewportOrigin, const glm::i32vec2& viewportSize);
	void setBackgroundColor(const glm::vec3& color);

	void bindInput();
	void unbindInput();

	void applyViewport() const;

	GlCameraPtr getCurrentCamera() const;
	GlCameraPtr addCamera();
	int getCameraCount() const;
	void setCurrentCamera(int cameraIndex);

	bool getCursorViewportLocation(glm::vec2& outViewportLocation) const;

	MulticastDelegate<void(const glm::vec3& rayOrigin, const glm::vec3& rayDir)> OnMouseRayChanged;
	MulticastDelegate<void(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)> OnMouseRayButtonDown;
	MulticastDelegate<void(const glm::vec3& rayOrigin, const glm::vec3& rayDir, int button)> OnMouseRayButtonUp;

protected:
	void onMouseMotion(int deltaX, int deltaY);
	void onMouseButtonDown(int button);
	void onMouseButtonUp(int button);
	void onMouseWheel(int scrollAmount);

private:
	bool m_bIsInputBound= false;
	bool m_isCameraRotateButtonPressed= false;

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
