#include "App.h"
#include "SdlCommon.h"
#include "MkMaterial.h"
#include "MkMaterialInstance.h"
#include "IMkTriangulatedMesh.h"
#include "IMkTexture.h"
#include "IMkShader.h"
#include "MikanShaderCache.h"
#include "IMkVertexDefinition.h"
#include "MainWindow.h"
#include "SteamVRRenderModelResource.h"
#include "ThreadUtils.h"

#include "openvr.h"

SteamVRRenderModelResource::SteamVRRenderModelResource(
	class IGlWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
	, m_steamVRRenderModel(nullptr)
	, m_steamVRTextureMap(nullptr)
	, m_glMesh(nullptr)
	, m_glDiffuseTexture(nullptr)
	, m_glMaterialInstance(nullptr)
{
}

SteamVRRenderModelResource::~SteamVRRenderModelResource()
{
	disposeSteamVRResources();
}

bool SteamVRRenderModelResource::createRenderResources()
{
	bool bSuccess= false;

	if (loadSteamVRResources())
	{
		m_glDiffuseTexture= createTextureResource(m_steamVRTextureMap);
		m_glMaterialInstance = createMaterialInstance(m_glDiffuseTexture);
		m_glMesh = createTriangulatedMeshResource(
			m_renderModelName, m_glMaterialInstance, m_steamVRRenderModel);

		bSuccess= (m_glDiffuseTexture != nullptr && m_glMaterialInstance != nullptr && m_glMesh != nullptr);
	}

	if (!bSuccess)
	{
		disposeRenderResources();
	}

	return bSuccess;
}

void SteamVRRenderModelResource::disposeRenderResources()
{
	m_glMesh= nullptr;
	m_glMaterialInstance= nullptr;
	m_glDiffuseTexture= nullptr;

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
	IMkTexturePtr glTexture = nullptr;

	if (steamvrTexture != nullptr)
	{
		glTexture = CreateMkTexture(
			steamvrTexture->unWidth,
			steamvrTexture->unHeight,
			steamvrTexture->rubTextureMapData,
			GL_RGBA,
			GL_RGBA);

		if (!glTexture->createTexture())
		{
			glTexture = nullptr;
		}
	}

	return glTexture;
}

GlMaterialInstancePtr SteamVRRenderModelResource::createMaterialInstance(
	IMkTexturePtr texture)
{
	auto* shaderCache= m_ownerWindow->getShaderCache();
	GlMaterialConstPtr material = shaderCache->getMaterialByName(INTERNAL_MATERIAL_PNT_TEXTURED_LIT_COLORED);
	assert(material != nullptr);
	GlMaterialInstancePtr materialInstance = std::make_shared<GlMaterialInstance>(material);

	// Fill in material parameter defaults
	if (texture)
	{
		materialInstance->setTextureBySemantic(eUniformSemantic::diffuseTexture, texture);
	}
	materialInstance->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, glm::vec4(1.f));

	return materialInstance;
}

IMkTriangulatedMeshPtr SteamVRRenderModelResource::createTriangulatedMeshResource(
	const std::string& meshName,
	GlMaterialInstancePtr materialInstance,
	const vr::RenderModel_t* steamVRRenderModel)
{
	IMkTriangulatedMeshPtr glMesh= nullptr;

	if (steamVRRenderModel != nullptr)
	{
		glMesh = createMkTriangulatedMesh(
			m_ownerWindow,
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
			glMesh= nullptr;
		}
	}

	return glMesh;
}
