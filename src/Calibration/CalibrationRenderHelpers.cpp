#include "App.h"
#include "GlViewport.h"
#include "ColorUtils.h"
#include "CalibrationRenderHelpers.h"
#include "GlLineRenderer.h"
#include "MathUtility.h"

GlLineRenderer* getLineRenderer()
{
	IGlWindow* window = App::getInstance()->getCurrentGlContext();
	assert(window != nullptr);

	return window->getLineRenderer();
}

GlLineRenderer* getLineRendererAndViewportBounds(
	float& outViewportX0, float& outViewportY0,
	float& outViewportX1, float& outViewportY1)
{
	IGlWindow* window = App::getInstance()->getCurrentGlContext();
	assert(window != nullptr);
	GlLineRenderer* lineRenderer = window->getLineRenderer();
	if (lineRenderer == nullptr)
		return nullptr;

	assert(window->getIsRenderingStage());
	GlViewportConstPtr viewport = window->getRenderingViewport();
	auto viewportOrigin = viewport->getViewportOrigin();
	auto viewportSize = viewport->getViewportSize();
	const float viewportWidth = viewportSize.x;
	const float viewportHeight = viewportSize.y;

	outViewportX0 = viewportOrigin.x;
	outViewportY0 = viewportOrigin.y;
	outViewportX1 = viewportOrigin.x + viewportWidth - 1.f;
	outViewportY1 = viewportOrigin.y + viewportHeight - 1.f;

	return lineRenderer;
}

glm::vec2 remapPointIntoTarget(
	const float sourceWidth, const float sourceHeight,
	const float targetX0, const float targetY0,
	const float targetX1, const float targetY1,
	const glm::vec2& sourcePoint)
{
	const float u = sourcePoint.x / sourceWidth;
	const float v = sourcePoint.y / sourceHeight;

	return glm::vec2(
		(1.f - u) * targetX0 + u * targetX1,
		(1.f - v) * targetY0 + v * targetY1);
}

void drawSegment2d(
	const float cameraWidth, const float cameraHeight,
	const glm::vec3& cameraSegmentStart, const glm::vec3& cameraSegmentEnd,
	const glm::vec3& colorStart, const glm::vec3& colorEnd)
{
	float viewportX0, viewportY0, viewportX1, viewportY1;
	GlLineRenderer* lineRenderer= getLineRendererAndViewportBounds(
		viewportX0, viewportY0,
		viewportX1, viewportY1);
	if (lineRenderer == nullptr)
		return;

	// Remaps the camera relative segment to window relative coordinates
	const glm::vec2 windowSegmentStart =
		remapPointIntoTarget(
			cameraWidth, cameraHeight,
			viewportX0, viewportY0,
			viewportX1, viewportY1,
			cameraSegmentStart);
	const glm::vec2 windowSegmentEnd =
		remapPointIntoTarget(
			cameraWidth, cameraHeight,
			viewportX0, viewportY0,
			viewportX1, viewportY1,
			cameraSegmentEnd);

	lineRenderer->addSegment2d(windowSegmentStart, colorStart, windowSegmentEnd, colorEnd);
}

void drawPointList2d(
	const float trackerWidth, const float trackerHeight,
	const glm::vec3* trackerPoints2D,
	const int trackerPointCount,
	const glm::vec3& color,
	const float point_size)
{
	float viewportX0, viewportY0, viewportX1, viewportY1;
	GlLineRenderer* lineRenderer = getLineRendererAndViewportBounds(
		viewportX0, viewportY0,
		viewportX1, viewportY1);
	if (lineRenderer == nullptr)
		return;

	for (int point_index = 0; point_index < trackerPointCount; ++point_index)
	{
		glm::vec2 windowPoint =
			remapPointIntoTarget(
				trackerWidth, trackerHeight,
				viewportX0, viewportY0,
				viewportX1, viewportY1,
				trackerPoints2D[point_index]);

		lineRenderer->addPoint2d(windowPoint, color, point_size);
	}
}

void drawQuadList2d(
	const float trackerWidth, const float trackerHeight,
	const float* trackerPoints2D,
	const int trackerPointCount,
	const glm::vec3& color)
{
	assert((trackerPointCount % 4) == 0);

	float viewportX0, viewportY0, viewportX1, viewportY1;
	GlLineRenderer* lineRenderer = getLineRendererAndViewportBounds(
		viewportX0, viewportY0,
		viewportX1, viewportY1);
	if (lineRenderer == nullptr)
		return;

	// Draw line strip connecting all of the points on the line strip
	for (int point_index = 0; point_index < trackerPointCount; point_index += 4) // 4 points per quad
	{
		int prev_vertex_index = 3;
		for (int vertex_index = 0; vertex_index < 4; ++vertex_index)
		{
			const glm::vec3 prevTrackerPoint(
				trackerPoints2D[2 * (point_index + prev_vertex_index)],     // x
				trackerPoints2D[2 * (point_index + prev_vertex_index) + 1], // y 
				0.5f);
			const glm::vec3 trackerPoint(
				trackerPoints2D[2 * (point_index + vertex_index)],     // x
				trackerPoints2D[2 * (point_index + vertex_index) + 1], // y
				0.5f);

			glm::vec2 prevWindowPoint =
				remapPointIntoTarget(
					trackerWidth, trackerHeight,
					viewportX0, viewportY0,
					viewportX1, viewportY1,
					prevTrackerPoint);
			glm::vec2 windowPoint =
				remapPointIntoTarget(
					trackerWidth, trackerHeight,
					viewportX0, viewportY0,
					viewportX1, viewportY1,
					trackerPoint);

			lineRenderer->addSegment2d(prevWindowPoint, color, windowPoint, color);

			prev_vertex_index = vertex_index;
		}
	}
}

void drawOpenCVChessBoard2D(
	const float trackerWidth, const float trackerHeight,
	const float* trackerPoints2d, const int trackerPointCount,
	bool validPoints)
{
	float viewportX0, viewportY0, viewportX1, viewportY1;
	GlLineRenderer* lineRenderer = getLineRendererAndViewportBounds(
		viewportX0, viewportY0,
		viewportX1, viewportY1);
	if (lineRenderer == nullptr)
		return;

	// Draw line strip connecting all of the corners on the chessboard
	{
		glm::vec3 prevColor;
		glm::vec2 prevWindowPoint;

		for (int sampleIndex = 0; sampleIndex < trackerPointCount; ++sampleIndex)
		{
			float r = 1.0f, g = 0.f, b = 0.f;
			if (validPoints)
			{
				// Match how OpenCV colors the line strip (red -> blue i.e. hue angle 0 to 255 degrees)
				const float hue = (float)(sampleIndex * 255 / trackerPointCount);

				HSVtoRGB(hue, 1.f, 1.f, r, g, b);
			}
			const glm::vec3 color(r, g, b);

			const glm::vec2 trackerPoint = glm::vec2(
				trackerPoints2d[sampleIndex * 2 + 0],
				trackerPoints2d[sampleIndex * 2 + 1]);
			const glm::vec2 windowPoint =
				remapPointIntoTarget(
					trackerWidth, trackerHeight,
					viewportX0, viewportY0,
					viewportX1, viewportY1,
					trackerPoint);

			if (sampleIndex > 0)
			{
				lineRenderer->addSegment2d(prevWindowPoint, prevColor, windowPoint, color);
			}

			prevWindowPoint = windowPoint;
			prevColor = color;
		}
	}

	// Draw circles at each corner
	for (int sampleIndex = 0; sampleIndex < trackerPointCount; ++sampleIndex)
	{
		const float radius = 2.f;
		const int subdiv = 8;
		const float angleStep = k_real_two_pi / static_cast<float>(subdiv);
		float angle = 0.f;

		float r = 1.0f, g = 0.f, b = 0.f;
		if (validPoints)
		{
			// Match how OpenCV colors the line strip (red -> blue i.e. hue angle 0 to 255 degrees)
			const float hue = (float)(sampleIndex * 255 / trackerPointCount);

			HSVtoRGB(hue, 1.f, 1.f, r, g, b);
		}
		glm::vec3 color(r, g, b);

		glm::vec2 prevWindowPoint;
		for (int index = 0; index <= subdiv; ++index)
		{
			glm::vec2 trackerPoint(
				radius * cosf(angle) + trackerPoints2d[sampleIndex * 2 + 0],
				radius * sinf(angle) + trackerPoints2d[sampleIndex * 2 + 1]);
			glm::vec2 windowPoint =
				remapPointIntoTarget(
					trackerWidth, trackerHeight,
					viewportX0, viewportY0,
					viewportX1, viewportY1,
					trackerPoint);

			if (index > 0)
			{
				lineRenderer->addSegment2d(prevWindowPoint, color, windowPoint, color);
			}

			angle += angleStep;
			prevWindowPoint = windowPoint;
		}
	}
}

void drawOpenCVChessBoard3D(
	const glm::mat4& xform,
	const glm::vec3* points3d, 
	const int pointCount,
	bool validPoints)
{
	GlLineRenderer* lineRenderer = getLineRenderer();
	if (lineRenderer == nullptr)
		return;

	// Draw line strip connecting all of the corners on the chessboard
	{
		glm::vec3 prevColor;
		glm::vec3 prevPoint;

		for (int sampleIndex = 0; sampleIndex < pointCount; ++sampleIndex)
		{
			float r = 1.0f, g = 0.f, b = 0.f;
			if (validPoints)
			{
				// Match how OpenCV colors the line strip (red -> blue i.e. hue angle 0 to 255 degrees)
				const float hue = (float)(sampleIndex * 255 / pointCount);

				HSVtoRGB(hue, 1.f, 1.f, r, g, b);
			}

			const glm::vec3 color(r, g, b);
			const glm::vec3 point = points3d[sampleIndex];

			if (sampleIndex > 0)
			{
				lineRenderer->addSegment3d(xform, prevPoint, prevColor, point, color);
			}

			prevPoint = point;
			prevColor = color;
		}
	}

	// Draw circles at each corner
	for (int sampleIndex = 0; sampleIndex < pointCount; ++sampleIndex)
	{
		const float radius = 0.005f;
		const int subdiv = 8;
		const float angleStep = k_real_two_pi / static_cast<float>(subdiv);
		float angle = 0.f;

		float r = 1.0f, g = 0.f, b = 0.f;
		if (validPoints)
		{
			// Match how OpenCV colors the line strip (red -> blue i.e. hue angle 0 to 255 degrees)
			const float hue = (float)(sampleIndex * 255 / pointCount);

			HSVtoRGB(hue, 1.f, 1.f, r, g, b);
		}

		const glm::vec3 center = points3d[sampleIndex];
		const glm::vec3 x_axis= glm::vec3(1.f, 0.f, 0.f);
		const glm::vec3 z_axis = glm::vec3(0.1, 0.f, 1.f);

		const glm::vec3 color(r, g, b);
		glm::vec3 prevCirclePoint;
		for (int index = 0; index <= subdiv; ++index)
		{
			const glm::vec3 circlePoint=
				center
				+ x_axis * radius * cosf(angle)
				+ z_axis * radius * sinf(angle);

			if (index > 0)
			{
				lineRenderer->addSegment3d(xform, prevCirclePoint, color, circlePoint, color);
			}

			angle += angleStep;
			prevCirclePoint = circlePoint;
		}
	}
}