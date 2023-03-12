#include "ColorUtils.h"
#include "CalibrationRenderHelpers.h"
#include "GlLineRenderer.h"
#include "MathUtility.h"
#include "Renderer.h"

glm::vec2 remapPointIntoSubWindow(
	const float screenWidth, const float screenHeight,
	const float windowLeft, const float windowTop,
	const float windowRight, const float windowBottom,
	const glm::vec2& in_point)
{
	const float u = in_point.x / screenWidth;
	const float v = in_point.y / screenHeight;

	return glm::vec2(
		(1.f - u) * windowLeft + u * windowRight,
		(1.f - v) * windowTop + v * windowBottom);
}

void drawSegment2d(
	const float cameraWidth, const float cameraHeight,
	const glm::vec3& cameraSegmentStart, const glm::vec3& cameraSegmentEnd,
	const glm::vec3& color)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	const float windowWidth = renderer->getSDLWindowWidth();
	const float windowHeight = renderer->getSDLWindowHeight();
	const float windowX0 = 0.0f, windowY0 = 0.f;
	const float windowX1 = windowWidth - 1.f, windowY1 = windowHeight - 1.f;

	// Remaps the camera relative segment to window relative coordinates
	const glm::vec2 windowSegmentStart =
		remapPointIntoSubWindow(
			cameraWidth, cameraHeight,
			windowX0, windowY0,
			windowX1, windowY1,
			cameraSegmentStart);
	const glm::vec2 windowSegmentEnd =
		remapPointIntoSubWindow(
			cameraWidth, cameraHeight,
			windowX0, windowY0,
			windowX1, windowY1,
			cameraSegmentEnd);

	renderer->getLineRenderer()->addSegment2d(windowSegmentStart, color, windowSegmentEnd, color);
}

void drawPointList2d(
	const float trackerWidth, const float trackerHeight,
	const glm::vec3* trackerPoints2D,
	const int trackerPointCount,
	const glm::vec3& color,
	const float point_size)
{
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	const float windowWidth = renderer->getSDLWindowWidth();
	const float windowHeight = renderer->getSDLWindowHeight();
	const float windowX0 = 0.0f, windowY0 = 0.f;
	const float windowX1 = windowWidth - 1.f, windowY1 = windowHeight - 1.f;

	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

	for (int point_index = 0; point_index < trackerPointCount; ++point_index)
	{
		glm::vec2 windowPoint =
			remapPointIntoSubWindow(
				trackerWidth, trackerHeight,
				windowX0, windowY0,
				windowX1, windowY1,
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

	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	const float windowWidth = renderer->getSDLWindowWidth();
	const float windowHeight = renderer->getSDLWindowHeight();
	const float windowX0 = 0.0f, windowY0 = 0.f;
	const float windowX1 = windowWidth - 1.f, windowY1 = windowHeight - 1.f;

	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

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
				remapPointIntoSubWindow(
					trackerWidth, trackerHeight,
					windowX0, windowY0,
					windowX1, windowY1,
					prevTrackerPoint);
			glm::vec2 windowPoint =
				remapPointIntoSubWindow(
					trackerWidth, trackerHeight,
					windowX0, windowY0,
					windowX1, windowY1,
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
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	const float windowWidth = renderer->getSDLWindowWidth();
	const float windowHeight = renderer->getSDLWindowHeight();
	const float windowX0 = 0.0f, windowY0 = 0.f;
	const float windowX1 = windowWidth - 1.f, windowY1 = windowHeight - 1.f;

	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

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
				remapPointIntoSubWindow(
					trackerWidth, trackerHeight,
					windowX0, windowY0,
					windowX1, windowY1,
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
				remapPointIntoSubWindow(
					trackerWidth, trackerHeight,
					windowX0, windowY0,
					windowX1, windowY1,
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
	Renderer* renderer = Renderer::getInstance();
	assert(renderer->getIsRenderingStage());
	GlLineRenderer* lineRenderer = renderer->getLineRenderer();

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