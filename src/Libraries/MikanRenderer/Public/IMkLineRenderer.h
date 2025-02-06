#pragma once

#include "MkRendererExport.h"
#include "MkRendererFwd.h"

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class IMkLineRenderer
{
public:
	virtual ~IMkLineRenderer() {}

	virtual bool startup() = 0;
	virtual void render() = 0;
	virtual void shutdown() = 0;

	virtual void setDisable3dDepth(bool bFlag) = 0;

	// Draw 3d points and lines in world space
	virtual void addPoint3d(
		const glm::mat4& xform,
		const glm::vec3& pos, const glm::vec3& color, 
		float size = 1.f) = 0;
	virtual void addSegment3d(
		const glm::mat4& xform,
		const glm::vec3& pos0, const glm::vec3& color0,
		const glm::vec3& pos1, const glm::vec3& color1,
		float size0 = 1.f, float size1 = 1.f) = 0;

	// Draw 2d points and lines in screen space [0,w-1]x[0,h-1]
	virtual void addPoint2d(
		const glm::vec2& pos, const glm::vec3& color,
		float size = 1.f) = 0;
	virtual void addSegment2d(
		const glm::vec2& pos0, const glm::vec3& color0,
		const glm::vec2& pos1, const glm::vec3& color1,
		float size0 = 1.f, float size1 = 1.f) = 0;
};

MIKAN_RENDERER_FUNC(IMkLineRendererPtr) CreateMkLineRenderer(IMkWindow* ownerWindow);