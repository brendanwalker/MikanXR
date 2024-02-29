#include "App.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlModelResourceManager.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlTexture.h"
#include "GlTriangulatedMesh.h"
#include "GlWireframeMesh.h"
#include "GlVertexDefinition.h"
#include "GlRenderModelResource.h"
#include "GlTextureCache.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4701) // potentially uninitialized local variable
#endif
#include "OBJ_Loader.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <limits>

namespace ObjUtils
{
	GlMaterialInstancePtr createTriMeshMaterialInstance(
		IGlWindow* ownerWindow,
		GlMaterialConstPtr material,
		const std::filesystem::path& modelFilePath,
		const objl::Material& objMaterial);
	GlTriangulatedMeshPtr createTriangulatedMeshResource(
		IGlWindow* ownerWindow,
		const std::string& meshName,
		const objl::Mesh& objMesh,
		GlMaterialInstancePtr materialInstance);
	GlWireframeMeshPtr createWireframeMeshResource(
		IGlWindow* ownerWindow,
		const std::string& meshName,
		const objl::Mesh& objMesh);
};

GlRenderModelResource::GlRenderModelResource(IGlWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
}

GlRenderModelResource::~GlRenderModelResource()
{
	disposeMeshRenderResources();
}

bool GlRenderModelResource::loadFromRenderModelFilePath(GlMaterialConstPtr overrideMaterial)
{
	if (m_ownerWindow == nullptr)
		return false;

	if (m_renderModelFilepath.empty())
		return false;

	GlMaterialConstPtr triMeshMaterial = overrideMaterial;
	if (!triMeshMaterial)
	{
		triMeshMaterial = m_ownerWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_BLINN_PHONG);
	}

	if (!triMeshMaterial)
		return false;

	// Material vertex definition must have position3f at a minimum
	const GlVertexDefinition& triMeshVertexDefinition= triMeshMaterial->getProgram()->getVertexDefinition();
	if (triMeshVertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::position3f) == nullptr)
	{
		return false;
	}

	disposeMeshRenderResources();

	std::string filepath = m_renderModelFilepath.string();
	// ObjLoader expects paths with forward slashes in path
	// If we don't convert, materials won't load correctly
	std::replace(filepath.begin(), filepath.end(), '\\', '/');

	// Create a new objl::Loader and load the file
	// TODO: Create a loader based on file type
	auto* m_objLoader = new objl::Loader();
	if (!m_objLoader->LoadFile(filepath))
	{
		return false;
	}
	
	for (const objl::Mesh& objMesh : m_objLoader->LoadedMeshes)
	{
		GlMaterialInstancePtr materialInstance =
			ObjUtils::createTriMeshMaterialInstance(
				m_ownerWindow,
				triMeshMaterial,
				m_renderModelFilepath,
				objMesh.MeshMaterial);
		GlTriangulatedMeshPtr glTriMesh =
			ObjUtils::createTriangulatedMeshResource(
				m_ownerWindow, objMesh.MeshName, objMesh, materialInstance);
		GlWireframeMeshPtr glWireframeMesh =
			ObjUtils::createWireframeMeshResource(
				m_ownerWindow, objMesh.MeshName, objMesh);

		addTriangulatedMesh(glTriMesh);
		addWireframeMesh(glWireframeMesh);
	}

	delete m_objLoader;
	m_objLoader = nullptr;

	return true;
}

bool GlRenderModelResource::saveToRenderModelFilePath() const
{
	if (m_renderModelFilepath.empty())
	{
		return false;
	}

	// TODO: Write out obj file
	// TODO: Write out mtl file
}

void GlRenderModelResource::addTriangulatedMesh(GlTriangulatedMeshPtr mesh)
{
	if (mesh != nullptr)
	{
		m_triangulatedMeshes.push_back(mesh);
	}
}

void GlRenderModelResource::addWireframeMesh(GlWireframeMeshPtr mesh)
{
	if (mesh != nullptr)
	{
		m_wireframeMeshes.push_back(mesh);
	}
}

void GlRenderModelResource::disposeMeshRenderResources()
{
	for (GlTriangulatedMeshPtr triMesh : m_triangulatedMeshes)
	{
		triMesh->deleteResources();
	}
	m_triangulatedMeshes.clear();

	for (GlWireframeMeshPtr wireframeMesh : m_wireframeMeshes)
	{
		wireframeMesh->deleteResources();
	}
	m_wireframeMeshes.clear();
}

namespace ObjUtils
{
	bool addTextureToMaterialInstance(
		GlTextureCache* textureCache,
		GlMaterialInstancePtr materialInstance,
		const std::filesystem::path& modelFilePath,
		const std::string& textureRelativePath,
		const std::string& uniformName)
	{
		if (!textureRelativePath.empty())
		{
			GlProgramPtr program = materialInstance->getMaterial()->getProgram();

			eUniformSemantic semantic;
			if (program->getUniformSemantic(uniformName, semantic) &&
				program->getUniformSemanticDataType(semantic) == eUniformDataType::datatype_texture)
			{
				std::filesystem::path texturePath = modelFilePath.parent_path() / textureRelativePath;
				GlTexturePtr texture = textureCache->loadTexturePath(texturePath);

				if (texture)
				{
					materialInstance->setTextureByUniformName(uniformName, texture);
					return true;
				}
			}
		}

		return false;
	}

	GlMaterialInstancePtr createTriMeshMaterialInstance(
		IGlWindow* ownerWindow,
		GlMaterialConstPtr material,
		const std::filesystem::path& modelFilePath,
		const objl::Material& objMaterial)
	{
		GlTextureCache* textureCache = ownerWindow->getTextureCache();
		GlMaterialInstancePtr materialInstance = std::make_shared<GlMaterialInstance>(material);

		materialInstance->setVec4BySemantic(
			eUniformSemantic::ambientColorRGBA,
			glm::vec4(objMaterial.Ka.X, objMaterial.Ka.Y, objMaterial.Ka.Z, 1.f));
		materialInstance->setVec4BySemantic(
			eUniformSemantic::diffuseColorRGBA,
			glm::vec4(objMaterial.Kd.X, objMaterial.Kd.Y, objMaterial.Kd.Z, 1.f));
		materialInstance->setVec4BySemantic(
			eUniformSemantic::specularColorRGBA,
			glm::vec4(objMaterial.Ks.X, objMaterial.Ks.Y, objMaterial.Ks.Z, 1.f));
		materialInstance->setFloatBySemantic(
			eUniformSemantic::shininess,
			objMaterial.Ns);

		// Ambient Texture Map
		addTextureToMaterialInstance(textureCache, materialInstance, modelFilePath, objMaterial.map_Ka, "map_Ka");
		// Diffuse Texture Map
		addTextureToMaterialInstance(textureCache, materialInstance, modelFilePath, objMaterial.map_Kd, "map_Kd");
		// Specular Texture Map
		addTextureToMaterialInstance(textureCache, materialInstance, modelFilePath, objMaterial.map_Ks, "map_Ks");
		// Specular Hightlight Map
		addTextureToMaterialInstance(textureCache, materialInstance, modelFilePath, objMaterial.map_Ns, "map_Ns");
		// Alpha Texture Map
		addTextureToMaterialInstance(textureCache, materialInstance, modelFilePath, objMaterial.map_d, "map_d");
		// Bump Map
		addTextureToMaterialInstance(textureCache, materialInstance, modelFilePath, objMaterial.map_bump, "map_bump");

		return materialInstance;
	}

	GlTriangulatedMeshPtr createTriangulatedMeshResource(
		IGlWindow* ownerWindow,
		const std::string& meshName,
		const objl::Mesh& objMesh,
		GlMaterialInstancePtr materialInstance)
	{
		if (materialInstance == nullptr || 
			objMesh.Indices.size() > std::numeric_limits<uint32_t>::max())
		{
			return GlTriangulatedMeshPtr();
		}

		GlMaterialConstPtr material = materialInstance->getMaterial();
		const GlVertexDefinition& vertexDefinition= material->getProgram()->getVertexDefinition();
		
		const size_t vertexCount = objMesh.Vertices.size();
		const size_t vertexSize = vertexDefinition.vertexSize;
		const size_t vertexBufferSize = vertexCount * vertexDefinition.vertexSize;
		uint8_t* vertexData = new uint8_t[vertexBufferSize];
		memset(vertexData, 0, vertexBufferSize);

		// Copy in vertex data requested by 
		for (const GlVertexAttribute& attrib : vertexDefinition.attributes)
		{
			uint8_t* writePtr = &vertexData[attrib.offset];

			switch (attrib.semantic)
			{
			case eVertexSemantic::position3f:
				{
					for (const objl::Vertex& vertex : objMesh.Vertices)
					{
						memcpy(writePtr, &vertex.Position, sizeof(float)*3);
						writePtr+= vertexSize;
					}
				} break;
			case eVertexSemantic::normal3f:
				{
					for (const objl::Vertex& vertex : objMesh.Vertices)
					{
						memcpy(writePtr, &vertex.Normal, sizeof(float) * 3);
						writePtr += vertexSize;
					}
				} break;
			case eVertexSemantic::texel2f:
				{
					for (const objl::Vertex& vertex : objMesh.Vertices)
					{
						memcpy(writePtr, &vertex.TextureCoordinate, sizeof(float) * 2);
						writePtr += vertexSize;
					}
				} break;
			}
		}		

		const size_t indexCount = objMesh.Indices.size();
		const size_t triangleCount = indexCount / 3;
		size_t indexSize = 0;
		uint8_t* indexBuffer = nullptr;

		// Use 32-bit indices
		if (indexCount > 65536)
		{
			const size_t indexBufferSize = indexCount * indexSize;
			indexBuffer = new uint8_t[indexBufferSize];

			// Straight copy of 32-bit indices
			std::memcpy(indexBuffer, objMesh.Indices.data(), indexBufferSize);
			indexSize = sizeof(uint32_t);
		}
		// Use 16-bit indices
		else
		{
			const size_t indexBufferSize = indexCount * indexSize;
			indexBuffer = new uint8_t[indexBufferSize];

			// Convert 32-bit index to 16-bit index
			uint16_t* writePtr = (uint16_t*)indexBuffer;
			for (const uint32_t index : objMesh.Indices)
			{
				*writePtr = (uint16_t)index;
				writePtr++;
			}

			indexSize = sizeof(uint16_t);
		}

		GlTriangulatedMeshPtr triMesh = std::make_shared<GlTriangulatedMesh>(
			ownerWindow,
			meshName,
			(const uint8_t*)vertexData,
			vertexSize,
			(uint32_t)vertexCount,
			(const uint8_t*)indexBuffer,
			indexSize,
			(uint32_t)triangleCount,
			true); // <-- triangulated mesh owns vertex data, cleans up on delete

		// If the mesh fails to set the material instance or create resources
		// then clean the mesh up and return nullptr
		if (!triMesh->setMaterialInstance(materialInstance) ||
			!triMesh->createResources())
		{
			triMesh = nullptr;
		}

		return triMesh;
	}

	GlWireframeMeshPtr createWireframeMeshResource(
		IGlWindow* ownerWindow,
		const std::string& meshName,
		const objl::Mesh& objMesh)
	{
		if (objMesh.Indices.size() > std::numeric_limits<uint32_t>::max())
		{
			return GlWireframeMeshPtr();
		}

		// Copy over the position data into the wireframe vertex buffer
		const size_t vertexCount = objMesh.Vertices.size();
		const size_t vertexSize = sizeof(float) * 3; // [x, y, z], [x, y, z], ...
		const size_t vertexBufferSize = vertexCount * vertexSize;
		uint8_t* vertexData = new uint8_t[vertexBufferSize];
	
		{
			uint8_t* writePtr= vertexData;

			for (const objl::Vertex& vertex : objMesh.Vertices)
			{
				memcpy(writePtr, &vertex, vertexSize);
				writePtr += vertexSize;
			}
		}

		// Convert the triangle index list into a line index list
		const size_t lineCount = objMesh.Indices.size(); // 3 indices per triangle, 3 lines per triangle
		const size_t indexCount = lineCount * 2;
		const size_t indexSize = sizeof(uint32_t);
		const size_t indexBufferSize = indexCount * indexSize;
		uint8_t* indexData = new uint8_t[indexBufferSize];

		{
			uint32_t* writePtr= (uint32_t *)indexData;

			for (size_t readIndex = 0; readIndex < objMesh.Indices.size(); readIndex+=3)
			{
				const uint32_t i0 = (uint32_t)objMesh.Indices[readIndex + 0];
				const uint32_t i1 = (uint32_t)objMesh.Indices[readIndex + 1];
				const uint32_t i2 = (uint32_t)objMesh.Indices[readIndex + 2];

				writePtr[0] = i0;
				writePtr[1] = i1;
				writePtr[2] = i1;
				writePtr[3] = i2;
				writePtr[4] = i2;
				writePtr[5] = i0;
				writePtr+= 6;
			}
		}

		GlWireframeMeshPtr wireMesh = std::make_shared<GlWireframeMesh>(
			ownerWindow,
			meshName,
			(const uint8_t*)vertexData,
			vertexSize,
			(uint32_t)vertexCount,
			(const uint8_t*)indexData,
			indexSize,
			(uint32_t)lineCount,
			true); // <-- wireframe mesh owns vertex data, cleans up on delete

		if (wireMesh->createResources())
		{
			auto matInstance = wireMesh->getMaterialInstance();

			matInstance->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, glm::vec4(Colors::Yellow, 1.f));
		}
		else
		{
			wireMesh = nullptr;
		}

		return wireMesh;
	}
};