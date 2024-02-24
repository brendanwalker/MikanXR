#include "App.h"
#include "GlCommon.h"
#include "GlMaterial.h"
#include "GlTriangulatedMesh.h"
#include "GlTexture.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlVertexDefinition.h"
#include "SteamVRRenderModelResource.h"
#include "ThreadUtils.h"

#include "openvr.h"

SteamVRRenderModelResource::SteamVRRenderModelResource(
	const std::string& renderModelName)
	: m_renderModelName(renderModelName)
	, m_steamVRRenderModel(nullptr)
	, m_steamVRTextureMap(nullptr)
	, m_glMesh(nullptr)
	, m_glDiffuseTexture(nullptr)
	, m_glMaterial(nullptr)
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
		m_glMaterial = createMaterial(getShaderCode(), m_glDiffuseTexture);
		m_glMesh = createTriangulatedMeshResource(
			m_renderModelName, getVertexDefinition(), m_steamVRRenderModel);

		bSuccess= (m_glDiffuseTexture != nullptr && m_glMaterial != nullptr && m_glMesh != nullptr);
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
	m_glMaterial= nullptr;
	m_glDiffuseTexture= nullptr;

	disposeSteamVRResources();
}

const GlProgramCode* SteamVRRenderModelResource::getShaderCode()
{
	static GlProgramCode x_shaderCode= GlProgramCode(
		"steamvr render model",
		// vertex shader
		R""""(
		#version 410 
		uniform mat4 matrix; 
		layout(location = 0) in vec4 position; 
		layout(location = 1) in vec3 v3NormalIn; 
		layout(location = 2) in vec2 v2TexCoordsIn; 
		out vec2 v2TexCoord; 
		void main() 
		{ 
			v2TexCoord = v2TexCoordsIn; 
			gl_Position = matrix * vec4(position.xyz, 1); 
		}
		)"""",
		//fragment shader
		R""""(
		#version 410 core
		uniform sampler2D diffuse;
		uniform vec4 modelColor;
		in vec2 v2TexCoord;
		out vec4 outputColor;
		void main()
		{
			outputColor = texture(diffuse, v2TexCoord) * modelColor;
		}
		)"""")
		.addUniform("matrix", eUniformSemantic::modelViewProjectionMatrix)
		.addUniform("diffuse", eUniformSemantic::texture0)
		.addUniform("modelColor", eUniformSemantic::diffuseColorRGBA);

	return &x_shaderCode;
}

const GlVertexDefinition* SteamVRRenderModelResource::getVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const int32_t vertexSize= (int32_t)sizeof(vr::RenderModel_Vertex_t);
		std::vector<GlVertexAttribute> &attribs= x_vertexDefinition.attributes;

		attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position3f, false, vertexSize, offsetof(vr::RenderModel_Vertex_t, vPosition)));
		attribs.push_back(GlVertexAttribute(1, eVertexSemantic::normal3f, false, vertexSize, offsetof(vr::RenderModel_Vertex_t, vNormal)));
		attribs.push_back(GlVertexAttribute(2, eVertexSemantic::texel2f, false, vertexSize, offsetof(vr::RenderModel_Vertex_t, rfTextureCoord)));

		x_vertexDefinition.vertexSize = vertexSize;
	}

	return &x_vertexDefinition;
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

GlTexturePtr SteamVRRenderModelResource::createTextureResource(
	const vr::RenderModel_TextureMap_t* steamvrTexture)
{
	GlTexturePtr glTexture = nullptr;

	if (steamvrTexture != nullptr)
	{
		glTexture = std::make_shared<GlTexture>(
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

GlMaterialPtr SteamVRRenderModelResource::createMaterial(
	const GlProgramCode* code,
	GlTexturePtr texture)
{
	GlShaderCache* resourceManager = GlShaderCache::getInstance();
	GlProgramPtr program = resourceManager->fetchCompiledGlProgram(code);
	
	if (program != nullptr)
	{
		const std::string materialName = m_renderModelName + "_" + code->getProgramName();
		GlMaterialPtr material = std::make_shared<GlMaterial>(materialName, program);

		// Fill in material parameter defaults
		if (texture)
		{
			material->setTextureBySemantic(eUniformSemantic::texture0, texture);
		}
		material->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, glm::vec4(1.f));

		return material;
	}
	else
	{
		return nullptr;
	}
}

GlTriangulatedMeshPtr SteamVRRenderModelResource::createTriangulatedMeshResource(
	const std::string& meshName,
	const GlVertexDefinition* vertexDefinition,
	const vr::RenderModel_t* steamVRRenderModel)
{
	GlTriangulatedMeshPtr glMesh= nullptr;

	if (steamVRRenderModel != nullptr)
	{
		glMesh = std::make_shared<GlTriangulatedMesh>(
			meshName,
			*vertexDefinition,
			(const uint8_t*)steamVRRenderModel->rVertexData,
			steamVRRenderModel->unVertexCount,
			(const uint8_t*)steamVRRenderModel->rIndexData,
			sizeof(uint16_t),
			steamVRRenderModel->unTriangleCount,
			false); // <-- triangulated mesh does not own vertex data

		if (!glMesh->createBuffers())
		{
			glMesh= nullptr;
		}
	}

	return glMesh;
}
