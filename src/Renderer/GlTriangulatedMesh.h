#pragma once

#include "IGlMesh.h"
#include "GlVertexDefinition.h"
#include "stdint.h"
#include <string>

class GlTriangulatedMesh : public IGlMesh
{
public:
	GlTriangulatedMesh() = default; 
	GlTriangulatedMesh(
		std::string name,
		const GlVertexDefinition& vertexDefintion,
		const uint8_t* vertexData,
		uint32_t vertexCount,
		const uint8_t* indexData,
		uint32_t triangleCount,
		bool bOwnsVertexData);
	virtual ~GlTriangulatedMesh();

	virtual void drawElements() const override;
	virtual bool createBuffers() override;
	virtual void deleteBuffers() override;

protected:
	std::string m_name;

	GlVertexDefinition m_vertexDefinition;

	const uint8_t* m_vertexData= nullptr;
	uint32_t m_vertexCount= 0;
	const uint8_t* m_indexData = nullptr;
	uint32_t m_triangleCount= 0;
	bool m_bOwnsVertexData= false;

	uint32_t m_glVertArray = 0;
	uint32_t m_glVertBuffer = 0;
	uint32_t m_glIndexBuffer = 0;
};
