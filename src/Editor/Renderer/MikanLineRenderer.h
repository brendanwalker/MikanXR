#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"

void drawPoint(const glm::mat4& transform, const glm::vec3& point, const glm::vec3& color, const float size);
void drawSegment(const glm::mat4& transform, const glm::vec3& start, const glm::vec3& end, const glm::vec3& color);
void drawSegment(const glm::mat4& transform, 
				 const glm::vec3& start, const glm::vec3& end, 
				 const glm::vec3& colorStart, const glm::vec3& colorEnd);
void drawArrow(const glm::mat4& transform, const glm::vec3& start, const glm::vec3& end, const float headFraction, const glm::vec3& color);
void drawGrid(const glm::mat4& transform, float xSize, float ySize, int xSubDiv, int ySubDiv, const glm::vec3& color);
void drawTransformedQuad(const glm::mat4& transform, float xSize, float ySize, const glm::vec3& color);
void drawTransformedCircle(const glm::mat4& transform, float radius, const glm::vec3& color);
void drawTransformedSpiralArc(
	const glm::mat4& transform,
	float radius,
	float radiusFractionPerCircle,
	float totalAngle,
	const glm::vec3& color);
void drawTransformedAxes(const glm::mat4& transform, float scale, bool drawLabels= false);
void drawTransformedAxes(const glm::mat4& transform, float xScale, float yScale, float zScale, bool drawLabels= false);
void drawTransformedAxes(
	const glm::mat4& transform,
	float xScale, float yScale, float zScale,
	const glm::vec3& xColor, const glm::vec3& yColor, const glm::vec3& zColor,
	bool drawLabels= false);
void drawTransformedTriangle(const glm::mat4& transform, const struct GlmTriangle& tri, const glm::vec3& color);
void drawTransformedBox(const glm::mat4& transform, const glm::vec3& half_extents, const glm::vec3& color);
void drawTransformedBox(const glm::mat4& transform, const glm::vec3& box_min, const glm::vec3& box_max, const glm::vec3& color);
void drawTransformedFrustum(
	const glm::mat4& transform,
	const float hfov_radians, const float vfov_radians,
	const float zNear, const float zFar,
	const glm::vec3& color);