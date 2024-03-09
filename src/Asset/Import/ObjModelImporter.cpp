#include "ObjModelImporter.h"
#include "Colors.h"
#include "IGlWindow.h"
#include "GlRenderModelResource.h"
#include "GlModelResourceManager.h"
#include "GlMaterialInstance.h"
#include "GlMaterial.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlTextureCache.h"
#include "GlTriangulatedMesh.h"
#include "GlWireframeMesh.h"

#include "fast_obj.h"

#include <glm/ext/vector_float4.hpp>

namespace ObjUtils
{
	class MaterialTriMeshData
	{
	public:
		MaterialTriMeshData(
			int materialId, 
			const std::string& materialName,
			GlMaterialInstancePtr materialInst) 
			: m_materialId(materialId)
			, m_materialName(materialName)
			, m_materialInstance(materialInst) 
		{
			GlMaterialConstPtr material = m_materialInstance->getMaterial();
			const GlVertexDefinition& vertexDefinition = material->getProgram()->getVertexDefinition();

			m_positionAttribute = vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::position);
			m_normalAttribute = vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::normal);
			m_texCoordAttribute = vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::texCoord);
			m_vertexSize = vertexDefinition.getVertexSize();

			assert(m_positionAttribute != nullptr && 
				   m_positionAttribute->getDataType() == eVertexDataType::datatype_vec3);
			assert(m_normalAttribute == nullptr || 
				   m_normalAttribute->getDataType() == eVertexDataType::datatype_vec3);
			assert(m_texCoordAttribute == nullptr || 
				   m_texCoordAttribute->getDataType() == eVertexDataType::datatype_vec2);
			assert(m_vertexSize > 0);
		}

		inline bool isValid() const 
		{ 
			return m_vertexCount > 0 && m_indexCount > 0; 
		}

		inline const std::string& getMaterialName() const { return m_materialName; }
		inline GlMaterialInstancePtr getMaterialInstance() const { return m_materialInstance; }
		const GlVertexAttribute* getPositionAttribute() const { return m_positionAttribute; }
		inline uint32_t getVertexCount() const { return m_vertexCount; }
		inline size_t getVertexSize() const { return m_vertexSize; }
		inline const uint8_t* getVertexData() const { return m_vertexData; }

		inline uint32_t getIndexCount() const { return m_indexCount; }
		inline const uint32_t* getIndexData() const { return m_indexData; }

		inline void incTriangleCount() { m_triangleCount++; }

		void allocateBuffers()
		{
			// Initially assume we need 3 vertices per triangle (max upper bound)
			// The final mesh will be compacted to the actual number of vertices
			const size_t estVertexCount = m_triangleCount*3;
			m_vertexData = new uint8_t[estVertexCount * m_vertexSize];
			m_indexData = new uint32_t[estVertexCount];

			// Remap buffer maps indices from the obj file's vertex list to the compacted vertex buffer
			m_indexRemapData = new int32_t[estVertexCount + 1];
			memset(m_indexRemapData, -1, sizeof(int32_t) * (estVertexCount + 1));
		}

		void addTriangle(const fastObjMesh& objData, const fastObjIndex* elementIndices)
		{
			for (int i= 0; i < 3; i++)
			{
				const fastObjIndex elementIndex = elementIndices[i];
				fastObjUInt sourcePositionIndex= elementIndex.p;
				assert(sourcePositionIndex != 0);

				uint32_t remappedIndex = m_indexRemapData[sourcePositionIndex];
				if (remappedIndex != -1)
				{
					// Re-use a vertex we already imported
					m_indexData[m_indexCount]= m_indexRemapData[sourcePositionIndex];
					m_indexCount++;
				}
				else
				{
					// Get the write pointer for the next vertex
					uint8_t* vertexWritePtr= &m_vertexData[m_vertexSize*m_vertexCount];

					// Copy in the position data
					glm::vec3 position(
						objData.positions[3 * elementIndex.p + 0],
						objData.positions[3 * elementIndex.p + 1],
						objData.positions[3 * elementIndex.p + 2]);
					memcpy(vertexWritePtr+m_positionAttribute->getOffset(), &position, sizeof(glm::vec3));

					// Copy in the normal data (if vertex has a normal)
					if (m_normalAttribute != nullptr)
					{
						glm::vec3 normal(
							objData.normals[3 * elementIndex.n + 0],
							objData.normals[3 * elementIndex.n + 1],
							objData.normals[3 * elementIndex.n + 2]);
						memcpy(vertexWritePtr + m_normalAttribute->getOffset(), &normal, sizeof(glm::vec3));
					}

					// Copy in the texel data (if vertex has a texel)
					if (m_texCoordAttribute != nullptr)
					{
						glm::vec2 texcoord(
							objData.texcoords[2 * elementIndex.t + 0],
							objData.texcoords[2 * elementIndex.t + 1]);
						memcpy(vertexWritePtr + m_texCoordAttribute->getOffset(), &texcoord, sizeof(glm::vec2));
					}

					// Map the obj vertex index to the new vertex index
					m_indexRemapData[sourcePositionIndex]= m_vertexCount;

					// Add the new vertex index to the index buffer
					m_indexData[m_indexCount] = m_vertexCount;
					m_indexCount++;

					// Finally, increment the vertex count
					m_vertexCount++;
				}
			}
		}

	private:
		int m_materialId= -1;
		std::string m_materialName;
		GlMaterialInstancePtr m_materialInstance;
		const GlVertexAttribute* m_positionAttribute= nullptr;
		const GlVertexAttribute* m_normalAttribute= nullptr;
		const GlVertexAttribute* m_texCoordAttribute= nullptr;
		uint8_t* m_vertexData= 0;
		size_t m_vertexSize= 0;
		uint32_t m_vertexCount= 0;
		uint32_t* m_indexData= 0;
		int32_t* m_indexRemapData = 0;
		uint32_t m_indexCount= 0;
		uint32_t m_triangleCount= 0;
	};
	using MaterialTriMeshDataPtr = std::shared_ptr<MaterialTriMeshData>;
	using MaterialTriMeshDataConstPtr = std::shared_ptr<const MaterialTriMeshData>;

	GlMaterialInstancePtr createTriMeshMaterialInstance(
		IGlWindow* ownerWindow,
		GlMaterialConstPtr material,
		const fastObjMaterial& objMaterial);
	GlTriangulatedMeshPtr createTriangulatedMeshResource(
		IGlWindow* ownerWindow,
		MaterialTriMeshDataConstPtr triMeshData);
	GlWireframeMeshPtr createWireframeMeshResource(
		IGlWindow* ownerWindow,
		MaterialTriMeshDataConstPtr triMeshData);
};

GlRenderModelResourcePtr ObjModelImporter::importModelFromFile(
	const std::filesystem::path& modelPath,
	GlMaterialConstPtr overrideMaterial)
{
	IGlWindow* ownerWindow= m_ownerManager->getOwnerWindow();
	GlShaderCache* shaderCache= ownerWindow->getShaderCache();

	GlRenderModelResourcePtr modelResource;

	if (modelPath.empty())
		return false;

	GlMaterialConstPtr triMeshMaterial = overrideMaterial;
	if (!triMeshMaterial)
	{
		triMeshMaterial = shaderCache->getMaterialByName(INTERNAL_MATERIAL_PNT_TEXTURED_LIT_COLORED);
	}

	if (!triMeshMaterial)
		return false;

	std::vector<ObjUtils::MaterialTriMeshDataPtr> materialToTrimeshMap;

	// Load the raw obj data
	std::string modelPathString = modelPath.string();
	fastObjMesh* objData = fast_obj_read(modelPathString.c_str());

	// Process the obj data into indexes triangles separated by material
	if (objData != nullptr)
	{
		// Create a new model resource
		modelResource = std::make_shared<GlRenderModelResource>(m_ownerManager->getOwnerWindow());

		// Create a material instance for each material in the obj file
		for (int materialIndex = 0; materialIndex < objData->material_count; materialIndex++)
		{
			const fastObjMaterial& objMaterial= objData->materials[materialIndex];
			const std::string materialName= objMaterial.name;

			GlMaterialInstancePtr materialInst= 
				ObjUtils::createTriMeshMaterialInstance(
					ownerWindow,
					triMeshMaterial,
					objMaterial);
			ObjUtils::MaterialTriMeshDataPtr triMeshData =
				std::make_shared<ObjUtils::MaterialTriMeshData>(
					materialIndex, materialName, materialInst);

			materialToTrimeshMap.push_back(triMeshData);
		}

		// Spin thru all the object faces counting up the number of triangles for each material
		for (uint32_t objectIndex = 0; objectIndex < objData->object_count; objectIndex++)
		{
			const fastObjGroup& group = objData->objects[objectIndex];

			// Count up the number of triangles for each material
			for (uint32_t faceIndex = 0; faceIndex < group.face_count; faceIndex++)
			{
				unsigned int faceVertexCount = objData->face_vertices[group.face_offset + faceIndex];
				unsigned int faceMaterialId = objData->face_materials[group.face_offset + faceIndex];

				if (faceVertexCount == 3 && faceMaterialId >= 0 && faceMaterialId < materialToTrimeshMap.size())
				{
					auto& triMeshData = materialToTrimeshMap[faceMaterialId];

					triMeshData->incTriangleCount();
				}
			}
		}

		// Allocate the vertex and index buffers for each material instance
		for (auto& triMeshData : materialToTrimeshMap)
		{
			triMeshData->allocateBuffers();
		};

		// Spin back all the object faces and actually add triangles to each material instance
		for (uint32_t objectIndex = 0; objectIndex < objData->object_count; objectIndex++)
		{
			const fastObjGroup& group = objData->objects[objectIndex];
			uint32_t groupElementIndex = 0;

			// Count up the number of triangles for each material
			for (uint32_t faceIndex = 0; faceIndex < group.face_count; faceIndex++)
			{
				unsigned int faceVertexCount = objData->face_vertices[group.face_offset + faceIndex];
				unsigned int faceMaterialId = objData->face_materials[group.face_offset + faceIndex];

				if (faceVertexCount == 3 && faceMaterialId >= 0 && faceMaterialId < materialToTrimeshMap.size())
				{
					auto& triMeshData = materialToTrimeshMap[faceMaterialId];
					const fastObjIndex* elementIndices = &objData->indices[group.index_offset + groupElementIndex];

					triMeshData->addTriangle(*objData, elementIndices);
				}

				groupElementIndex+= faceVertexCount;
			}
		}

		// Create the meshes for each material instance
		for (ObjUtils::MaterialTriMeshDataPtr triMeshData : materialToTrimeshMap)
		{
			GlTriangulatedMeshPtr trimesh= ObjUtils::createTriangulatedMeshResource(ownerWindow, triMeshData);
			GlWireframeMeshPtr wiremesh= ObjUtils::createWireframeMeshResource(ownerWindow, triMeshData);

			modelResource->addTriangulatedMesh(trimesh);
			modelResource->addWireframeMesh(wiremesh);
		}
	}

	if (objData != nullptr)
	{
		fast_obj_destroy(objData);
	}

	return modelResource;
}

namespace ObjUtils
{
	bool addTextureToMaterialInstance(
		GlTextureCache* textureCache,
		GlMaterialInstancePtr materialInstance,
		const fastObjTexture& objTexture,
		const eUniformSemantic semantic)
	{
		GlProgramPtr program = materialInstance->getMaterial()->getProgram();

		// See if the shader has a uniform that wants to bind to a texture with the given semantic
		std::string uniformName;
		if (program->getFirstUniformNameOfSemantic(semantic, uniformName) &&
			program->getUniformSemanticDataType(semantic) == eUniformDataType::datatype_texture)
		{
			// Try loading the texture using the relative path
			GlTexturePtr texture;
			if (objTexture.path != nullptr && objTexture.path[0] != '\0')
			{
				texture = textureCache->loadTexturePath(objTexture.path);
			}

			// If that fails, fallback to the default white texture
			if (texture == nullptr)
			{
				texture = textureCache->tryGetTextureByName(INTERNAL_TEXTURE_WHITE);
			}

			if (texture)
			{
				// Bind the texture to the material instance
				return materialInstance->setTextureByUniformName(uniformName, texture);
			}
			else
			{
				// Failed to load any texture to bind to the uniform
				return false;
			}
		}
		else
		{
			// The shader doesn't have uniform that cares about this texture
			return true;
		}
	}

	GlMaterialInstancePtr createTriMeshMaterialInstance(
		IGlWindow* ownerWindow,
		GlMaterialConstPtr material,
		const fastObjMaterial& objMaterial)
	{
		GlTextureCache* textureCache = ownerWindow->getTextureCache();
		GlMaterialInstancePtr materialInstance = std::make_shared<GlMaterialInstance>(material);

		materialInstance->setVec3BySemantic(
			eUniformSemantic::ambientColorRGB,
			glm::vec4(objMaterial.Ka[0], objMaterial.Ka[1], objMaterial.Ka[2], 1.f));
		materialInstance->setVec3BySemantic(
			eUniformSemantic::diffuseColorRGBA,
			glm::vec4(objMaterial.Kd[0], objMaterial.Kd[1], objMaterial.Kd[2], 1.f));
		materialInstance->setVec3BySemantic(
			eUniformSemantic::specularColorRGB,
			glm::vec4(objMaterial.Ks[0], objMaterial.Ks[1], objMaterial.Ks[2], 1.f));
		materialInstance->setFloatBySemantic(
			eUniformSemantic::specularHighlights,
			objMaterial.Ns);
		materialInstance->setFloatBySemantic(
			eUniformSemantic::opticalDensity,
			objMaterial.Ni);
		materialInstance->setFloatBySemantic(
			eUniformSemantic::dissolve,
			objMaterial.d);

		// Ambient Texture Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, objMaterial.map_Ka, eUniformSemantic::ambientTexture);
		// Diffuse Texture Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, objMaterial.map_Kd, eUniformSemantic::diffuseTexture);
		// Specular Texture Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, objMaterial.map_Ks, eUniformSemantic::specularTexture);
		// Specular Hightlight Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, objMaterial.map_Ns, eUniformSemantic::specularHightlightTexture);
		// Alpha Texture Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, objMaterial.map_d, eUniformSemantic::alphaTexture);
		// Bump Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, objMaterial.map_bump, eUniformSemantic::bumpTexture);

		return materialInstance;
	}


	GlTriangulatedMeshPtr createTriangulatedMeshResource(
		IGlWindow* ownerWindow,
		MaterialTriMeshDataConstPtr triMeshData)
	{
		if (!triMeshData->isValid())
		{
			return GlTriangulatedMeshPtr();
		}

		// Copy over the vertex data into a correctly size vertex buffer
		const size_t vertexCount = triMeshData->getVertexCount();
		const size_t vertexSize = triMeshData->getVertexSize();
		const size_t vertexBufferSize = vertexCount * vertexSize;
		uint8_t* vertexBuffer = new uint8_t[vertexBufferSize];
		memcpy(vertexBuffer, triMeshData->getVertexData(), vertexBufferSize);

		// Copy over the index data into a correctly size index buffer
		const size_t indexCount = triMeshData->getIndexCount();
		const size_t triangleCount = indexCount / 3;
		const size_t indexSize = sizeof(uint32_t);
		const size_t indexBufferSize = indexCount * indexSize;
		uint8_t* indexBuffer = new uint8_t[indexBufferSize];
		std::memcpy(indexBuffer, triMeshData->getIndexData(), indexBufferSize);

		GlTriangulatedMeshPtr triMesh = std::make_shared<GlTriangulatedMesh>(
			ownerWindow,
			triMeshData->getMaterialName(),
			(const uint8_t*)vertexBuffer,
			vertexSize,
			(uint32_t)vertexCount,
			(const uint8_t*)indexBuffer,
			indexSize,
			(uint32_t)triangleCount,
			true); // <-- triangulated mesh owns vertex data, cleans up on delete

		// If the mesh fails to set the material instance or create resources
		// then clean the mesh up and return nullptr
		if (!triMesh->setMaterialInstance(triMeshData->getMaterialInstance()) ||
			!triMesh->createResources())
		{
			triMesh = nullptr;
		}

		return triMesh;
	}

	GlWireframeMeshPtr createWireframeMeshResource(
		IGlWindow* ownerWindow,
		MaterialTriMeshDataConstPtr triMeshData)
	{
		if (!triMeshData->isValid())
		{
			return GlWireframeMeshPtr();
		}

		// Copy over the position data into the wireframe vertex buffer
		const GlVertexAttribute* posAttribute = triMeshData->getPositionAttribute();
		const size_t posAttribSize = posAttribute->getAttributeSize();

		const size_t writeVertexSize = posAttribSize; // [x, y, z], [x, y, z], ...
		const size_t writeVertexCount = triMeshData->getVertexCount();
		const size_t writeVertexBufferSize = writeVertexCount * writeVertexSize;
		uint8_t* writeVertexData = new uint8_t[writeVertexBufferSize];

		{
			const size_t readVertexSize= triMeshData->getVertexSize();
			const uint8_t* readPtr = triMeshData->getVertexData() + posAttribute->getOffset();

			uint8_t* writePtr = writeVertexData + posAttribute->getOffset();

			for (size_t vertexIndex= 0; vertexIndex < writeVertexCount; ++vertexIndex)
			{
				memcpy(writePtr, readPtr, posAttribSize);
				readPtr += readVertexSize;
				writePtr += writeVertexSize;
			}
		}

		// Convert the triangle index list into a line index list
		const size_t lineCount = triMeshData->getIndexCount(); // 3 indices per triangle, 3 lines per triangle
		const size_t indexCount = lineCount * 2;
		const size_t indexSize = sizeof(uint32_t);
		const size_t indexBufferSize = indexCount * indexSize;
		uint8_t* indexData = new uint8_t[indexBufferSize];

		{
			const size_t triangleCount = triMeshData->getIndexCount() / 3;
			const uint32_t* readPtr = triMeshData->getIndexData();
			uint32_t* writePtr = (uint32_t*)indexData;

			for (size_t triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++)
			{
				const uint32_t i0 = readPtr[0];
				const uint32_t i1 = readPtr[1];
				const uint32_t i2 = readPtr[2];
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

		GlWireframeMeshPtr wireMesh = std::make_shared<GlWireframeMesh>(
			ownerWindow,
			triMeshData->getMaterialName(),
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

			matInstance->setVec4BySemantic(eUniformSemantic::diffuseColorRGBA, glm::vec4(Colors::Yellow, 1.f));
		}
		else
		{
			wireMesh = nullptr;
		}

		return wireMesh;
	}
};