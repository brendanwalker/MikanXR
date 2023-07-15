#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

glm::vec2 remapPointIntoSubWindow(
	const float screenWidth, const float screenHeight,
	const float windowLeft, const float windowTop,
	const float windowRight, const float windowBottom,
	const glm::vec2& in_point);

void drawSegment2d(
	const float cameraWidth, const float cameraHeight,
	const glm::vec3& cameraSegmentStart, const glm::vec3& cameraSegmentEnd,
	const glm::vec3& colorStart, const glm::vec3& colorEnd);

void drawPointList2d(
	const float trackerWidth, const float trackerHeight,
	const glm::vec3* trackerPoints2D,
	const int trackerPointCount,
	const glm::vec3& color,
	const float point_size);

void drawQuadList2d(
	const float trackerWidth, const float trackerHeight,
	const float* point_data_2d,
	const int point_count,
	const glm::vec3& color);

void drawOpenCVChessBoard2D(
	const float trackerWidth, const float trackerHeight,
	const float* points2d, const int point_count,
	bool validPoints);

void drawOpenCVChessBoard3D(
	const glm::mat4& xform,
	const glm::vec3* points3d,
	const int pointCount,
	bool validPoints);
