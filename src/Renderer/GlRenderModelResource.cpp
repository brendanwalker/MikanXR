#include "App.h"
#include "GlCommon.h"
#include "GlMaterial.h"
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
	delete m_vertexDefinition;
}

bool GlRenderModelResource::createRenderResources()
{
	bool bSuccess = false;

	if (loadObjFileResources())
	{
		for (const objl::Mesh& mesh : m_objLoader->LoadedMeshes)
		{
			GlTriangulatedMesh* glMesh = 
				createTriangulatedMeshResource(
					mesh.MeshName, getVertexDefinition(), &mesh);
			GlWireframeMesh* glWireframeMesh =
				createWireframeMeshResource(
					mesh.MeshName, &mesh);

			if (glMesh != nullptr)
			{
				m_glMeshes.push_back(glMesh);
			}

			if (glWireframeMesh != nullptr)
			{
				m_glWireframeMeshes.push_back(glWireframeMesh);
			}
		}

		bSuccess = m_glMeshes.size() > 0;
	}

	if (!bSuccess)
	{
		disposeRenderResources();
	}

	return bSuccess;
}

void GlRenderModelResource::disposeRenderResources()
{
	for (GlTriangulatedMesh* glMesh : m_glMeshes)
	{
		glMesh->deleteBuffers();
		delete glMesh;
	}
	m_glMeshes.clear();

	for (GlWireframeMesh* glWireframeMesh : m_glWireframeMeshes)
	{
		glWireframeMesh->deleteBuffers();
		delete glWireframeMesh;
	}
	m_glWireframeMeshes.clear();

	disposeObjFileResources();
}

bool GlRenderModelResource::loadObjFileResources()
{
	if (m_renderModelFilepath.empty())
	{
		return false;
	}

	disposeRenderResources();

	m_objLoader = new objl::Loader();
	if (!m_objLoader->LoadFile(m_renderModelFilepath.string()))
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

GlTriangulatedMesh* GlRenderModelResource::createTriangulatedMeshResource(
	const std::string& meshName,
	const GlVertexDefinition* vertexDefinition,
	const objl::Mesh* objMesh)
{
	GlTriangulatedMesh* glMesh = nullptr;

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

		glMesh = new GlTriangulatedMesh(
			meshName,
			*vertexDefinition,
			(const uint8_t*)vertexData,
			(uint32_t)vertexCount,
			(const uint8_t*)indexBuffer,
			(uint32_t)triangleCount,
			true); // <-- triangulated mesh owns vertex data, cleans up on delete

		if (!glMesh->createBuffers())
		{
			delete glMesh;
			glMesh = nullptr;
		}
	}

	return glMesh;
}

GlWireframeMesh* GlRenderModelResource::createWireframeMeshResource(
	const std::string& meshName,
	const objl::Mesh* objMesh)
{
	GlWireframeMesh* glWireframeMesh = nullptr;

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

		glWireframeMesh = new GlWireframeMesh(
			meshName,
			(const uint8_t*)vertexData,
			(uint32_t)vertexCount,
			(const uint8_t*)indexData,
			(uint32_t)lineCount,
			true); // <-- wireframe mesh owns vertex data, cleans up on delete

		if (!glWireframeMesh->createBuffers())
		{
			delete glWireframeMesh;
			glWireframeMesh = nullptr;
		}
	}

	return glWireframeMesh;
}