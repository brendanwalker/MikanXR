#pragma once

#include "GlVertexDefinition.h"
#include "stdint.h"
#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class GlWireframeMesh
{
public:
	GlWireframeMesh() = default;
	GlWireframeMesh(
		std::string name,
		const uint8_t* vertexData,
		uint32_t vertexCount,
		const uint8_t* indexData,
		uint32_t lineCount,
		bool bOwnsVertexData);
	virtual ~GlWireframeMesh();

	inline class GlProgram* getDefaultWireframeShader() const { return m_program; }

	void drawElements() const;
	bool createBuffers();
	void deleteBuffers();

protected:
	static const class GlProgramCode* getShaderCode();
	static const struct GlVertexDefinition* getVertexDefinition();

	std::string m_name;

	GlVertexDefinition m_vertexDefinition;

	const uint8_t* m_vertexData = nullptr;
	uint32_t m_vertexCount = 0;
	const uint8_t* m_indexData = nullptr;
	uint32_t m_lineCount = 0;
	bool m_bOwnsVertexData = false;

	uint32_t m_glVertArray = 0; 
	uint32_t m_glVertBuffer = 0;
	uint32_t m_glIndexBuffer = 0;
	class GlProgram* m_program = nullptr;
};

void drawTransformedWireframeMesh(
	const glm::mat4& transform,
	const class GlWireframeMesh* wireframeMesh,
	const glm::vec3& color);