#include "App.h"
#include "Colors.h"
#include "GlCommon.h"
#include "GlMaterial.h"
#include "GlMaterialInstance.h"
#include "GlModelResourceManager.h"
#include "GlTriangulatedMesh.h"
#include "GlWireframeMesh.h"
#include "GlVertexDefinition.h"
#include "GlRenderModelResource.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable: 4701) // potentially uninitialized local variable
#endif
#include "OBJ_Loader.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <limits>

GlRenderModelResource::GlRenderModelResource()
	: m_vertexDefinition(nullptr)
{

}

GlRenderModelResource::GlRenderModelResource(const GlVertexDefinition* vertexDefinition)
	: m_vertexDefinition(new GlVertexDefinition(*vertexDefinition))
{

}

GlRenderModelResource::GlRenderModelResource(const std::filesystem::path& modelFilePath)
	: m_renderModelFilepath(modelFilePath)
	, m_vertexDefinition(new GlVertexDefinition(*getVertexDefinition()))
{
}

GlRenderModelResource::GlRenderModelResource(
	const std::filesystem::path& modelFilePath,
	const GlVertexDefinition* vertexDefinition)
	: m_renderModelFilepath(modelFilePath)
	, m_vertexDefinition(new GlVertexDefinition(*vertexDefinition))
{
}

GlRenderModelResource::~GlRenderModelResource()
{
	disposeRenderResources();
	if (m_vertexDefinition)
	{
		delete m_vertexDefinition;
	}
}

const GlVertexDefinition* GlRenderModelResource::getDefaultVertexDefinition()
{
	static GlVertexDefinition x_vertexDefinition;

	if (x_vertexDefinition.attributes.size() == 0)
	{
		const uint32_t positionSize = (uint32_t)sizeof(float) * 3;
		const uint32_t normalSize = (uint32_t)sizeof(float) * 3;
		const uint32_t texelSize = (uint32_t)sizeof(float) * 2;
		const uint32_t vertexSize = positionSize + normalSize + texelSize;
		std::vector<GlVertexAttribute>& attribs = x_vertexDefinition.attributes;

		size_t offset= 0;
		attribs.push_back(GlVertexAttribute(0, eVertexSemantic::position3f, false, vertexSize, offset));
		offset+= positionSize;

		attribs.push_back(GlVertexAttribute(1, eVertexSemantic::normal3f, false, vertexSize, offset));
		offset+= normalSize;

		attribs.push_back(GlVertexAttribute(2, eVertexSemantic::texel2f, false, vertexSize, offset));
		offset+= texelSize;

		assert(offset == vertexSize);
		x_vertexDefinition.vertexSize = vertexSize;
	}

	return &x_vertexDefinition;
}

bool GlRenderModelResource::createRenderResources(GlModelResourceManager* modelResourceManager)
{
	bool bSuccess = false;

	if (loadObjFileResources())
	{
		for (const objl::Mesh& mesh : m_objLoader->LoadedMeshes)
		{
			GlTriangulatedMeshPtr glTriMesh = 
				createTriangulatedMeshResource(
					mesh.MeshName, getVertexDefinition(), &mesh);
			GlMaterialInstancePtr glTriMeshMaterial =
				createTriMeshMaterialInstance(
					modelResourceManager->getPhongMaterial(), 
					&mesh.MeshMaterial);
			GlWireframeMeshPtr glWireframeMesh =
				createWireframeMeshResource(
					mesh.MeshName, &mesh);
			GlMaterialInstancePtr glWireframeMeshMaterial =
				createWireframeMeshMaterialInstance(
					modelResourceManager->getWireframeMaterial());

			if (glTriMesh != nullptr)
			{
				m_glTriMeshResources.push_back({glTriMesh, glTriMeshMaterial});
			}

			if (glWireframeMesh != nullptr)
			{
				m_glWireframeMeshResources.push_back({glWireframeMesh, glWireframeMeshMaterial});
			}
		}

		bSuccess = m_glTriMeshResources.size() > 0;
	}

	if (!bSuccess)
	{
		disposeRenderResources();
	}

	return bSuccess;
}

void GlRenderModelResource::disposeRenderResources()
{
	for (TriMeshResourceEntry& glMeshResource : m_glTriMeshResources)
	{
		glMeshResource.glMesh->deleteBuffers();
	}
	m_glTriMeshResources.clear();

	for (WireframeMeshResourceEntry glWireframeMesh : m_glWireframeMeshResources)
	{
		glWireframeMesh.glMesh->deleteBuffers();
	}
	m_glWireframeMeshResources.clear();

	disposeObjFileResources();
}

bool GlRenderModelResource::loadObjFileResources()
{
	if (m_renderModelFilepath.empty())
	{
		return false;
	}

	disposeRenderResources();

	std::string filepath= m_renderModelFilepath.string();
	// ObjLoader expects paths with forward slashes in path
	// If we don't convert, materials won't load correctly
	std::replace( filepath.begin(), filepath.end(), '\\', '/' );

	m_objLoader = new objl::Loader();
	if (!m_objLoader->LoadFile(filepath))
	{
		return false;
	}

	return true;
}

void GlRenderModelResource::disposeObjFileResources()
{
	if (m_objLoader != nullptr)
	{
		delete m_objLoader;
		m_objLoader= nullptr;
	}
}

GlTriangulatedMeshPtr GlRenderModelResource::createTriangulatedMeshResource(
	const std::string& meshName,
	const GlVertexDefinition* vertexDefinition,
	const objl::Mesh* objMesh)
{
	GlTriangulatedMeshPtr glMesh = nullptr;	

	if (objMesh != nullptr && objMesh->Indices.size() <= std::numeric_limits<uint16_t>::max())
	{
		const size_t vertexCount = objMesh->Vertices.size();
		const size_t vertexSize = vertexDefinition->vertexSize;
		const size_t vertexBufferSize = vertexCount * vertexDefinition->vertexSize;
		uint8_t* vertexData = new uint8_t[vertexBufferSize];
		memset(vertexData, 0, vertexBufferSize);

		// Copy in vertex data requested by 
		for (const GlVertexAttribute& attrib : vertexDefinition->attributes)
		{
			uint8_t* writePtr = &vertexData[attrib.offset];

			switch (attrib.semantic)
			{
			case eVertexSemantic::position3f:
				{
					for (const objl::Vertex& vertex : objMesh->Vertices)
					{
						memcpy(writePtr, &vertex.Position, sizeof(float)*3);
						writePtr+= vertexSize;
					}
				} break;
			case eVertexSemantic::normal3f:
				{
					for (const objl::Vertex& vertex : objMesh->Vertices)
					{
						memcpy(writePtr, &vertex.Normal, sizeof(float) * 3);
						writePtr += vertexSize;
					}
				} break;
			case eVertexSemantic::texel2f:
				{
					for (const objl::Vertex& vertex : objMesh->Vertices)
					{
						memcpy(writePtr, &vertex.TextureCoordinate, sizeof(float) * 2);
						writePtr += vertexSize;
					}
				} break;
			}
		}		

		// Convert 32-bit index to 16-bit index
		const size_t indexCount = objMesh->Indices.size();
		const size_t triangleCount = indexCount / 3;
		const size_t indexSize = sizeof(uint16_t);
		const size_t indexBufferSize = indexCount * indexSize;
		uint8_t* indexBuffer = new uint8_t[indexBufferSize];

		{
			uint16_t* writePtr = (uint16_t*)indexBuffer;

			for (const uint32_t index : objMesh->Indices)
			{
				*writePtr = (uint16_t)index;
				writePtr++;
			}
		}

		glMesh = std::make_shared<GlTriangulatedMesh>(
			meshName,
			*vertexDefinition,
			(const uint8_t*)vertexData,
			(uint32_t)vertexCount,
			(const uint8_t*)indexBuffer,
			indexSize,
			(uint32_t)triangleCount,
			true); // <-- triangulated mesh owns vertex data, cleans up on delete

		if (!glMesh->createBuffers())
		{
			glMesh = nullptr;
		}
	}

	return glMesh;
}

GlMaterialInstancePtr GlRenderModelResource::createTriMeshMaterialInstance(
	GlMaterialConstPtr material,
	const objl::Material* objMaterial)
{
	//GlMaterialConstPtr phongMaterial= GlModelResourceManager::getInstance()->getPhongMaterial();
	GlMaterialInstancePtr glMaterialInstance = std::make_shared<GlMaterialInstance>(material);

	glMaterialInstance->setVec4BySemantic(
		eUniformSemantic::ambientColorRGBA, 
		glm::vec4(objMaterial->Ka.X, objMaterial->Ka.Y, objMaterial->Ka.Z, 1.f));
	glMaterialInstance->setVec4BySemantic(
		eUniformSemantic::diffuseColorRGBA,
		glm::vec4(objMaterial->Kd.X, objMaterial->Kd.Y, objMaterial->Kd.Z, 1.f));
	glMaterialInstance->setVec4BySemantic(
		eUniformSemantic::specularColorRGBA,
		glm::vec4(objMaterial->Ks.X, objMaterial->Ks.Y, objMaterial->Ks.Z, 1.f));
	glMaterialInstance->setFloatBySemantic(
		eUniformSemantic::shininess,
		objMaterial->Ns);

	return glMaterialInstance;
}

GlWireframeMeshPtr GlRenderModelResource::createWireframeMeshResource(
	const std::string& meshName,
	const objl::Mesh* objMesh)
{
	GlWireframeMeshPtr glWireframeMesh = nullptr;

	if (objMesh != nullptr && objMesh->Indices.size() <= std::numeric_limits<uint16_t>::max())
	{
		// Copy over the position data into the wireframe vertex buffer
		const size_t vertexCount = objMesh->Vertices.size();
		const size_t vertexByteStride = sizeof(float) * 3; // [x, y, z], [x, y, z], ...
		const size_t vertexBufferSize = vertexCount * vertexByteStride;
		uint8_t* vertexData = new uint8_t[vertexBufferSize];
	
		{
			uint8_t* writePtr= vertexData;

			for (const objl::Vertex& vertex : objMesh->Vertices)
			{
				memcpy(writePtr, &vertex, vertexByteStride);
				writePtr += vertexByteStride;
			}
		}

		// Convert the triangle index list into a line index list
		const size_t lineCount = objMesh->Indices.size(); // 3 indices per triangle, 3 lines per triangle
		const size_t indexCount = lineCount * 2;
		const size_t indexByteStride = sizeof(uint16_t);
		const size_t indexBufferSize = indexCount * indexByteStride;
		uint8_t* indexData = new uint8_t[indexBufferSize];

		{
			uint16_t* writePtr= (uint16_t *)indexData;

			for (size_t readIndex = 0; readIndex < objMesh->Indices.size(); readIndex+=3)
			{
				const uint16_t i0 = (uint16_t)objMesh->Indices[readIndex + 0];
				const uint16_t i1 = (uint16_t)objMesh->Indices[readIndex + 1];
				const uint16_t i2 = (uint16_t)objMesh->Indices[readIndex + 2];

				writePtr[0] = i0;
				writePtr[1] = i1;
				writePtr[2] = i1;
				writePtr[3] = i2;
				writePtr[4] = i2;
				writePtr[5] = i0;
				writePtr+= 6;
			}
		}

		glWireframeMesh = std::make_shared<GlWireframeMesh>(
			meshName,
			(const uint8_t*)vertexData,
			(uint32_t)vertexCount,
			(const uint8_t*)indexData,
			(uint32_t)lineCount,
			true); // <-- wireframe mesh owns vertex data, cleans up on delete

		if (!glWireframeMesh->createBuffers())
		{
			glWireframeMesh = nullptr;
		}
	}

	return glWireframeMesh;
}

GlMaterialInstancePtr GlRenderModelResource::createWireframeMeshMaterialInstance(
	GlMaterialConstPtr wireframeMaterial)
{
	GlMaterialInstancePtr glMaterialInstance = std::make_shared<GlMaterialInstance>(wireframeMaterial);

	glMaterialInstance->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, glm::vec4(Colors::Yellow, 1.f));

	return glMaterialInstance;
}