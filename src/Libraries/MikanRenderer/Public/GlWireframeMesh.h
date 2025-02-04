#pragma once

#include "IGlMesh.h"
#include "GlVertexDefinition.h"
#include "RendererFwd.h"

#include "stdint.h"

#include <memory>
#include <string>

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class GlWireframeMesh : public IGlMesh
{
public:
	GlWireframeMesh(class IGlWindow* ownerWindow);
	GlWireframeMesh(
		class IGlWindow* ownerWindow,
		std::string name,
		const uint8_t* vertexData,
		const size_t vertexSize,
		uint32_t vertexCount,
		const uint8_t* indexData,
		const size_t indexSize,
		uint32_t lineCount,
		bool bOwnsVertexData);
	virtual ~GlWireframeMesh();

	virtual std::string getName() const override { return m_name; }
	virtual std::shared_ptr<class GlMaterialInstance> getMaterialInstance() const { return m_materialInstance; };
	virtual class IGlWindow* getOwnerWindow() const { return m_ownerWindow; }
	virtual const uint8_t* getVertexData() const override { return m_vertexData; }
	virtual const uint32_t getVertexCount() const override { return m_vertexCount; }

	virtual const uint8_t* getIndexData() const override { return m_indexData; }
	virtual const size_t getElementCount() const override { return m_lineCount; }
	virtual const size_t getIndexPerElementCount() const override { return 2; }
	virtual const size_t getIndexSize() const override { return m_indexSize; }

	virtual void drawElements() const override;
	virtual bool createResources() override;
	virtual void deleteResources() override;

protected:
	class IGlWindow* m_ownerWindow = nullptr;
	GlMaterialInstancePtr m_materialInstance;

	std::string m_name;

	const uint8_t* m_vertexData = nullptr;
	uint32_t m_vertexCount = 0;
	size_t m_vertexSize= 0;
	const uint8_t* m_indexData = nullptr;
	size_t m_indexSize = 0;
	uint32_t m_lineCount = 0;
	bool m_bOwnsVertexData = false;

	uint32_t m_glVertArray = 0; 
	uint32_t m_glVertBuffer = 0;
	uint32_t m_glIndexBuffer = 0;
};

void drawTransformedWireframeMesh(
	GlCameraConstPtr camera,
	const glm::mat4& transform,
	const class GlWireframeMesh* wireframeMesh,
	const glm::vec3& color);