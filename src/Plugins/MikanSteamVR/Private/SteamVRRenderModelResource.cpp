#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "IMkTriangulatedMesh.h"
#include "IMkWireframeMesh.h"
#include "IMkTexture.h"
#include "IMkShader.h"
#include "IMkShaderCache.h"
#include "IMkGraphicsContext.h"
#include "IMkVertexDefinition.h"
#include "SteamVRRenderModelResource.h"
#include "ThreadUtils.h"

#include "glm/ext/vector_float4.hpp"
#include "openvr.h"

SteamVRRenderModelResource::SteamVRRenderModelResource(
	class IMkGraphicsContext* ownerContext)
	: m_ownerContext(ownerContext)
	, m_steamVRRenderModel(nullptr)
	, m_steamVRTextureMap(nullptr)
	, m_mkTriangulatedMesh(nullptr)
	, m_mkDiffuseTexture(nullptr)
	, m_mkMaterialInstance(nullptr)
{}

SteamVRRenderModelResource::~SteamVRRenderModelResource()
{
	disposeSteamVRResources();
}

bool SteamVRRenderModelResource::createGraphicsResources()
{
	if (m_steamVRRenderModel == nullptr || m_steamVRTextureMap == nullptr)
		return false;

	m_mkDiffuseTexture = createTextureResource(m_steamVRTextureMap);
	m_mkMaterialInstance = createMaterialInstance(m_mkDiffuseTexture);
	m_mkTriangulatedMesh = createTriangulatedMeshResource(
		m_renderModelName, m_mkMaterialInstance, m_steamVRRenderModel);
	m_mkWireframeMesh = createWireframeMeshResource(
		m_renderModelName, m_steamVRRenderModel);

	const bool bSuccess =
		m_mkDiffuseTexture != nullptr &&
		m_mkMaterialInstance != nullptr &&
		m_mkTriangulatedMesh != nullptr &&
		m_mkWireframeMesh != nullptr;

	if (!bSuccess)
	{
		disposeRenderResources();
	}

	return bSuccess;
}

void SteamVRRenderModelResource::disposeRenderResources()
{
	m_mkTriangulatedMesh = nullptr;
	m_mkWireframeMesh = nullptr;
	m_mkMaterialInstance = nullptr;
	m_mkDiffuseTexture = nullptr;

	disposeSteamVRResources();
}


bool SteamVRRenderModelResource::loadSteamVRResources()
{
	if (m_renderModelName.size() == 0)
	{
		return false;
	}

	while (m_steamVRRenderModel == nullptr)
	{
		vr::EVRRenderModelError result =
			vr::VRRenderModels()->LoadRenderModel_Async(m_renderModelName.c_str(), &m_steamVRRenderModel);

		if (result == vr::VRRenderModelError_Loading)
		{
			ThreadUtils::sleepMilliseconds(1);
			continue;
		}
		else if (result != vr::VRRenderModelError_None)
		{
			return false;
		}
	}

	while (m_steamVRTextureMap == nullptr)
	{
		vr::EVRRenderModelError result =
			vr::VRRenderModels()->LoadTexture_Async(
				m_steamVRRenderModel->diffuseTextureId,
				&m_steamVRTextureMap);

		if (result == vr::VRRenderModelError_Loading)
		{
			ThreadUtils::sleepMilliseconds(1);
			continue;
		}
		else if (result != vr::VRRenderModelError_None)
		{
			return false;
		}
	}

	return true;
}

void SteamVRRenderModelResource::disposeSteamVRResources()
{
	if (m_steamVRTextureMap != nullptr)
	{
		if (vr::VRRenderModels() != nullptr)
			vr::VRRenderModels()->FreeTexture(m_steamVRTextureMap);
		m_steamVRTextureMap = nullptr;
	}

	if (m_steamVRRenderModel != nullptr)
	{
		if (vr::VRRenderModels() != nullptr)
			vr::VRRenderModels()->FreeRenderModel(m_steamVRRenderModel);
		m_steamVRRenderModel = nullptr;
	}
}

IMkTexturePtr SteamVRRenderModelResource::createTextureResource(
	const vr::RenderModel_TextureMap_t* steamvrTexture)
{
	IMkTexturePtr mkTexture = nullptr;

	if (steamvrTexture != nullptr)
	{
		mkTexture = CreateMkTexture(
			steamvrTexture->unWidth,
			steamvrTexture->unHeight,
			steamvrTexture->rubTextureMapData,
			MK_RGBA,
			MK_RGBA);

		if (!mkTexture->createTexture())
		{
			mkTexture = nullptr;
		}
	}

	return mkTexture;
}

MkMaterialInstancePtr SteamVRRenderModelResource::createMaterialInstance(
	IMkTexturePtr texture)
{
	IMkShaderCache* shaderCache = m_ownerContext->getShaderCache();
	MkMaterialConstPtr material = shaderCache->getMaterialByName(INTERNAL_MATERIAL_PNT_TEXTURED_LIT_COLORED);
	assert(material != nullptr);
	MkMaterialInstancePtr materialInstance = createMkMaterialInstance(material);

	// Fill in material parameter defaults
	if (texture)
	{
		materialInstance->setTextureBySemantic(eUniformSemantic::diffuseTexture, texture);
	}
	materialInstance->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, glm::vec4(1.f));
	materialInstance->setFloatBySemantic(eUniformSemantic::ambientStrength, 0.3f);
	materialInstance->setVec3BySemantic(eUniformSemantic::specularColorRGB, glm::vec3(0.5f));
	materialInstance->setFloatBySemantic(eUniformSemantic::specularHighlights, 32.f);

	return materialInstance;
}

IMkTriangulatedMeshPtr SteamVRRenderModelResource::createTriangulatedMeshResource(
	const std::string& meshName,
	MkMaterialInstancePtr materialInstance,
	const vr::RenderModel_t* steamVRRenderModel)
{
	IMkTriangulatedMeshPtr glMesh = nullptr;

	if (steamVRRenderModel != nullptr)
	{
		glMesh = createMkTriangulatedMesh(
			m_ownerContext,
			meshName,
			(const uint8_t*)steamVRRenderModel->rVertexData,
			sizeof(vr::RenderModel_Vertex_t),
			steamVRRenderModel->unVertexCount,
			(const uint8_t*)steamVRRenderModel->rIndexData,
			sizeof(uint16_t),
			steamVRRenderModel->unTriangleCount,
			false); // <-- triangulated mesh does not own vertex data

		if (!glMesh->setMaterialInstance(materialInstance) ||
			!glMesh->createResources())
		{
			glMesh = nullptr;
		}
	}

	return glMesh;
}

IMkWireframeMeshPtr SteamVRRenderModelResource::createWireframeMeshResource(
	const std::string& meshName,
	const vr::RenderModel_t* steamVRRenderModel)
{
	if (steamVRRenderModel == nullptr)
	{
		return IMkWireframeMeshPtr();
	}

	// Copy over the position data into the wireframe vertex buffer
	const size_t posAttribSize = sizeof(vr::HmdVector3_t);

	const size_t writeVertexSize = posAttribSize; // [x, y, z], [x, y, z], ...
	const size_t writeVertexCount = steamVRRenderModel->unVertexCount;
	const size_t writeVertexBufferSize = writeVertexCount * writeVertexSize;
	uint8_t* writeVertexData = new uint8_t[writeVertexBufferSize];

	{
		const size_t readVertexSize = sizeof(vr::RenderModel_Vertex_t);
		const uint8_t* readPtr = 
			(const uint8_t*)steamVRRenderModel->rVertexData + 
			offsetof(vr::RenderModel_Vertex_t, vPosition);

		uint8_t* writePtr = writeVertexData;

		for (size_t vertexIndex = 0; vertexIndex < writeVertexCount; ++vertexIndex)
		{
			memcpy(writePtr, readPtr, posAttribSize);
			readPtr += readVertexSize;
			writePtr += writeVertexSize;
		}
	}

	// Convert the triangle index list into a line index list
	const size_t lineCount = steamVRRenderModel->unTriangleCount * 3; // 3 lines per triangle
	const size_t indexCount = lineCount * 2;
	const size_t indexSize = sizeof(uint32_t);
	const size_t indexBufferSize = indexCount * indexSize;
	uint8_t* indexData = new uint8_t[indexBufferSize];

	{
		const size_t triangleCount = steamVRRenderModel->unTriangleCount;
		const uint16_t* readPtr = steamVRRenderModel->rIndexData;
		uint32_t* writePtr = (uint32_t*)indexData;

		for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
		{
			const uint32_t i0 = (uint32_t)readPtr[0];
			const uint32_t i1 = (uint32_t)readPtr[1];
			const uint32_t i2 = (uint32_t)readPtr[2];
			readPtr += 3;

			writePtr[0] = i0;
			writePtr[1] = i1;
			writePtr[2] = i1;
			writePtr[3] = i2;
			writePtr[4] = i2;
			writePtr[5] = i0;
			writePtr += 6;
		}
	}

	IMkWireframeMeshPtr wireMesh = CreateMkWireframeMesh(
		m_ownerContext,
		meshName,
		(const uint8_t*)writeVertexData,
		writeVertexSize,
		(uint32_t)writeVertexCount,
		(const uint8_t*)indexData,
		indexSize,
		(uint32_t)lineCount,
		true); // <-- wireframe mesh owns vertex data, cleans up on delete

	if (wireMesh->createResources())
	{
		auto matInstance = wireMesh->getMaterialInstance();

		matInstance->setVec4BySemantic(
			eUniformSemantic::diffuseColorRGBA, 
			glm::vec4(1.f, 1.f, 0.f, 1.f)); // yellow
	}
	else
	{
		wireMesh = IMkWireframeMeshPtr();
	}

	return wireMesh;
}