#pragma once

#include "MkRendererFwd.h"

#include "memory"
#include "vector"

#include "glm/ext/vector_int2_sized.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

class IMkViewport
{
public:
	virtual ~IMkViewport() {}

	virtual glm::i32vec2 getViewportOrigin() const = 0;
	virtual glm::i32vec2 getViewportSize() const = 0;

	virtual void setViewport(const glm::i32vec2& viewportOrigin, const glm::i32vec2& viewportSize) = 0;
	virtual void setBackgroundColor(const glm::vec3& color) = 0;

	virtual void applyRenderingViewport(IMkState* glState) const = 0;
	virtual void onRenderingViewportApply(int x, int y, int width, int height) = 0;
	virtual void onRenderingViewportRevert(int x, int y, int width, int height) = 0;
	virtual bool getRenderingViewport(glm::i32vec2 &outOrigin, glm::i32vec2 &outSize) const = 0;

	virtual IMkCameraPtr getCurrentCamera() const = 0;
	virtual int getCurrentCameraIndex() const = 0;
	virtual IMkCameraPtr addCamera() = 0;
	virtual int getCameraCount() const = 0;
	virtual IMkCameraPtr getCameraByIndex(int cameraIndex) = 0;
	virtual void setCurrentCamera(int cameraIndex) = 0;
};
