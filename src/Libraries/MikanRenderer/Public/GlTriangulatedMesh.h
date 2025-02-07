#pragma once

#include "IMkMesh.h"
#include "MikanRendererFwd.h"

#include "stdint.h"
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class GlTriangulatedMesh : public IMkMesh
{
public:
	GlTriangulatedMesh(class IMkWindow* ownerWindow); 
	GlTriangulatedMesh(
		class IMkWindow* ownerWindow,
		std::string name,
		const uint8_t* vertexData,
		const size_t vertexSize,
		uint32_t vertexCount,
		const uint8_t* indexData,
		const size_t indexSize,
		uint32_t triangleCount,
		bool bOwnsVertexData);
	virtual ~GlTriangulatedMesh();

	bool setMaterial(GlMaterialConstPtr material);
	bool setMaterialInstance(GlMaterialInstancePtr materialInstance);

	virtual void drawElements() const override;
	virtual bool createResources() override;
	virtual void deleteResources() override;

	virtual std::string getName() const override { return m_name; }
	virtual std::shared_ptr<class GlMaterialInstance> getMaterialInstance() const { return m_materialInstance; };
	virtual class IMkWindow* getOwnerWindow() const { return m_ownerWindow; }
	virtual const uint8_t* getVertexData() const override { return m_vertexData; }
	virtual const uint32_t getVertexCount() const override { return m_vertexCount; }

	virtual const uint8_t* getIndexData() const override { return m_indexData; }
	virtual const size_t getElementCount() const override { return m_triangleCount; }
	virtual const size_t getIndexPerElementCount() const override { return 3; }
	virtual const size_t getIndexSize() const override { return m_indexSize; }

protected:
	class IMkWindow* m_ownerWindow= nullptr;
	GlMaterialInstancePtr m_materialInstance;
	std::string m_name;

	const uint8_t* m_vertexData= nullptr;
	size_t m_vertexSize= 0;
	uint32_t m_vertexCount= 0;
	const uint8_t* m_indexData = nullptr;
	const size_t m_indexSize = sizeof(uint16_t);
	uint32_t m_triangleCount= 0;
	bool m_bOwnsVertexData= false;

	uint32_t m_glVertArray = 0;
	uint32_t m_glVertBuffer = 0;
	uint32_t m_glIndexBuffer = 0;
};

// -- Drawing Helpers ---
GlTriangulatedMeshPtr createFullscreenQuadMesh(IMkWindow* ownerWindow, bool vFlipped);
void drawTransformedTriangulatedMesh(
	GlCameraConstPtr camera,
	const glm::mat4& transform,
	GlTriangulatedMeshConstPtr wireframeMesh);