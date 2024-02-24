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
		const size_t indexSize,
		uint32_t triangleCount,
		bool bOwnsVertexData);
	virtual ~GlTriangulatedMesh();

	virtual void drawElements() const override;
	virtual bool createBuffers() override;
	virtual void deleteBuffers() override;

	virtual std::string getName() const override { return m_name; }
	virtual const GlVertexDefinition* getVertexDefinition() const override { return &m_vertexDefinition; }
	virtual const uint8_t* getVertexData() const override { return m_vertexData; }
	virtual const uint32_t getVertexCount() const override { return m_vertexCount; }

	virtual const uint8_t* getIndexData() const override { return m_indexData; }
	virtual const size_t getElementCount() const override { return m_triangleCount; }
	virtual const size_t getIndexPerElementCount() const override { return 3; }
	virtual const size_t getIndexSize() const override { return m_indexSize; }

protected:
	std::string m_name;

	GlVertexDefinition m_vertexDefinition;

	const uint8_t* m_vertexData= nullptr;
	uint32_t m_vertexCount= 0;
	const uint8_t* m_indexData = nullptr;
	const size_t m_indexSize = sizeof(uint16_t);
	uint32_t m_triangleCount= 0;
	bool m_bOwnsVertexData= false;

	uint32_t m_glVertArray = 0;
	uint32_t m_glVertBuffer = 0;
	uint32_t m_glIndexBuffer = 0;
};
