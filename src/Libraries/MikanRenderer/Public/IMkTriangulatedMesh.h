#pragma once

#include "IMkMesh.h"
#include "MkRendererFwd.h"
#include "MkRendererExport.h"

#include "stdint.h"
#include <string>

#include "glm/ext/matrix_float4x4.hpp"

class IMkTriangulatedMesh : public IMkMesh
{
public:
	virtual bool setMaterial(GlMaterialConstPtr material) = 0;
	virtual bool setMaterialInstance(GlMaterialInstancePtr materialInstance) = 0;
};

// -- Drawing Helpers ---
MIKAN_RENDERER_FUNC(IMkTriangulatedMeshPtr) createMkTriangulatedMesh(class IMkWindow* ownerWindow);
MIKAN_RENDERER_FUNC(IMkTriangulatedMeshPtr) createMkTriangulatedMesh(
	class IMkWindow* ownerWindow,
	std::string name,
	const uint8_t* vertexData,
	const size_t vertexSize,
	uint32_t vertexCount,
	const uint8_t* indexData,
	const size_t indexSize,
	uint32_t triangleCount,
	bool bOwnsVertexData);
MIKAN_RENDERER_FUNC(IMkTriangulatedMeshPtr) createFullscreenQuadMesh(IMkWindow* ownerWindow, bool vFlipped);
MIKAN_RENDERER_FUNC(void) drawTransformedTriangulatedMesh(
	IMkCameraConstPtr camera,
	const glm::mat4& transform,
	IMkTriangulatedMeshConstPtr wireframeMesh);