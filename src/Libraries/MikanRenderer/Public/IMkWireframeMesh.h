#pragma once

#include "MkRendererExport.h"
#include "MkRendererFwd.h"
#include "IMkMesh.h"

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class IMkWireframeMesh : public IMkMesh
{
public:
};

MIKAN_RENDERER_FUNC(IMkWireframeMeshPtr) CreateMkWireframeMesh(class IMkWindow* ownerWindow);
MIKAN_RENDERER_FUNC(IMkWireframeMeshPtr) CreateMkWireframeMesh(
	class IMkWindow* ownerWindow,
	std::string name,
	const uint8_t* vertexData,
	const size_t vertexSize,
	uint32_t vertexCount,
	const uint8_t* indexData,
	const size_t indexSize,
	uint32_t lineCount,
	bool bOwnsVertexData);

MIKAN_RENDERER_FUNC(void) drawTransformedWireframeMesh(
	IMkCameraConstPtr camera,
	const glm::mat4& transform,
	const class IMkWireframeMesh* wireframeMesh,
	const glm::vec3& color);