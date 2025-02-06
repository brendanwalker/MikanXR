#pragma once

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"

#include <memory>
#include <string>

class GlProgram;
typedef std::shared_ptr<GlProgram> GlProgramPtr;

class GlLineRenderer
{
public:
	GlLineRenderer(class IMkWindow* m_ownerWindow);
	virtual ~GlLineRenderer();

	bool startup();
	void render();
	void shutdown();

	void setDisable3dDepth(bool bFlag) { m_bDisable3dDepth= bFlag; }

	// Draw 3d points and lines in world space
	void addPoint3d(
		const glm::mat4& xform,
		const glm::vec3& pos, const glm::vec3& color, 
		float size = 1.f);
	void addSegment3d(
		const glm::mat4& xform,
		const glm::vec3& pos0, const glm::vec3& color0,
		const glm::vec3& pos1, const glm::vec3& color1,
		float size0 = 1.f, float size1 = 1.f);

	// Draw 2d points and lines in screen space [0,w-1]x[0,h-1]
	void addPoint2d(
		const glm::vec2& pos, const glm::vec3& color,
		float size = 1.f);
	void addSegment2d(
		const glm::vec2& pos0, const glm::vec3& color0,
		const glm::vec2& pos1, const glm::vec3& color1,
		float size0 = 1.f, float size1 = 1.f);

protected:
	class IMkWindow* m_ownerWindow= nullptr;

	static const class GlProgramCode* getShaderCode();

	struct Point
	{
		glm::vec3 position;
		glm::vec4 colorAndSize;
	};

	class PointBufferState
	{
	public:
		PointBufferState(int maxPoints);
		virtual ~PointBufferState();

		void createGlBufferState(GlProgramPtr program);
		void drawGlBufferState(unsigned int glEnumMode);
		void destroyGlBufferState();

		inline bool hasPoints() const { return m_pointCount > 0; }
		void addPoint3d(
			const glm::mat4& xform,
			const glm::vec3& pos,
			const glm::vec3& color,
			float size);
		void addPoint2d(
			const glm::vec2& pos,
			const glm::vec3& color,
			float size);

	private:
		Point* m_points;
		int m_maxPoints;
		int m_pointCount;
		unsigned int m_pointVAO;
		unsigned int m_pointVBO;
	};

	GlProgramPtr m_program = nullptr;
	std::string m_modelViewUniformName;

	PointBufferState m_points3d;
	PointBufferState m_lines3d;
	PointBufferState m_points2d;
	PointBufferState m_lines2d;

	bool m_bDisable3dDepth = false;

};
//-- drawing methods -----
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