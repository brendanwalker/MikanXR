#pragma once

#include "IGlMesh.h"
#include "GlVertexDefinition.h"
#include "stdint.h"

#include <memory>
#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class GlProgram;
typedef std::shared_ptr<GlProgram> GlProgramPtr;

class GlWireframeMesh : public IGlMesh
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

	inline GlProgramPtr getDefaultWireframeShader() const { return m_program; }

	virtual const GlVertexDefinition* getVertexDefinition() const override { return getVertexDefinitionInternal(); }
	virtual const uint8_t* getVertexData() const override { return m_vertexData; }
	virtual const uint32_t getVertexCount() const override { return m_vertexCount; }

	virtual const uint8_t* getIndexData() const override { return m_indexData; }
	virtual const size_t getElementCount() const override { return m_lineCount; }
	virtual const size_t getIndexPerElementCount() const override { return 2; }
	virtual const size_t getIndexSize() const override { return sizeof(uint16_t); }

	virtual void drawElements() const override;
	virtual bool createBuffers() override;
	virtual void deleteBuffers() override;

protected:
	static const class GlProgramCode* getShaderCode();
	static const struct GlVertexDefinition* getVertexDefinitionInternal();

	std::string m_name;

	const uint8_t* m_vertexData = nullptr;
	uint32_t m_vertexCount = 0;
	const uint8_t* m_indexData = nullptr;
	uint32_t m_lineCount = 0;
	bool m_bOwnsVertexData = false;

	uint32_t m_glVertArray = 0; 
	uint32_t m_glVertBuffer = 0;
	uint32_t m_glIndexBuffer = 0;
	GlProgramPtr m_program = nullptr;
};

void drawTransformedWireframeMesh(
	const glm::mat4& transform,
	const class GlWireframeMesh* wireframeMesh,
	const glm::vec3& color);