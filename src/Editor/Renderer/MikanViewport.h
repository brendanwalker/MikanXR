#pragma once

#include "MulticastDelegate.h"
#include "MikanRendererFwd.h"
#include "IMkViewport.h"

#include "memory"
#include "vector"

#include "glm/ext/vector_int2_sized.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

class MikanViewport : public std::enable_shared_from_this<MikanViewport>, public IMkViewport
{
public:
	MikanViewport(const glm::i32vec2& windowSize);
	virtual ~MikanViewport();

	virtual glm::i32vec2 getViewportOrigin() const override { return m_viewportOrigin; }
	virtual glm::i32vec2 getViewportSize() const override { return m_viewportSize; }

	virtual void setViewport(const glm::i32vec2& viewportOrigin, const glm::i32vec2& viewportSize) override;
	virtual void setBackgroundColor(const glm::vec3& color) override;

	void bindInput();
	void unbindInput();

	virtual void applyRenderingViewport(GlState& glState) const override;
	virtual void onRenderingViewportApply(int x, int y, int width, int height) override;
	virtual void onRenderingViewportRevert(int x, int y, int width, int height) override;
	virtual bool getRenderingViewport(glm::i32vec2 &outOrigin, glm::i32vec2 &outSize) const override;

	void update(float deltaSeconds);

	virtual IMkCameraPtr getCurrentCamera() const override;
	virtual int getCurrentCameraIndex() const override;
	virtual IMkCameraPtr addCamera() override;
	virtual int getCameraCount() const override;
	virtual IMkCameraPtr getCameraByIndex(int cameraIndex) override;
	virtual void setCurrentCamera(int cameraIndex) override;
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
	glm::i32vec2 m_renderOrigin;
	glm::i32vec2 m_renderSize;
	glm::vec4 m_backgroundColor;

	std::vector<GlCameraPtr> m_cameraPool;
	int m_currentCameraIndex= 0;

	const float k_camera_mouse_zoom_scalar = 0.1f;
	const float k_camera_mouse_pan_scalar = 0.25f;
	const float k_camera_min_zoom = 0.01f;
};
