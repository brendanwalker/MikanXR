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
#include "Logger.h"
#include "SdlUtility.h"
#include "Version.h"

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
	bool writeMaterialInstanceTextureToFile(
		GlMaterialInstanceConstPtr materialInstance,
		const std::filesystem::path& mtlPath,
		const eUniformSemantic semantic,
		std::string& outRelativeTexturePath);
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
		triMeshMaterial = m_ownerWindow->getShaderCache()->getMaterialByName(INTERNAL_MATERIAL_PNT_TEXTURED_LIT_COLORED);
	}

	if (!triMeshMaterial)
		return false;

	// Material vertex definition must have position3f at a minimum
	const GlVertexDefinition& triMeshVertexDefinition= triMeshMaterial->getProgram()->getVertexDefinition();
	if (triMeshVertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::position) == nullptr)
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

	const std::string dir= m_renderModelFilepath.parent_path().string();
	const std::string stem= m_renderModelFilepath.stem().string();
	const std::filesystem::path mtlPath= m_renderModelFilepath.parent_path() / (stem + ".mtl");

	{
		std::ofstream objFile(m_renderModelFilepath);
		std::ofstream mtlFile(mtlPath);
		if (!objFile.is_open() || !mtlFile.is_open())
		{
			return false;
		}

		// Write out the obj header
		objFile << "# Mikan: " << MIKAN_RELEASE_VERSION_STRING << std::endl;
		objFile << "mtllib " << stem << ".mtl" << std::endl;

		// Write out the mtl header
		mtlFile << "# Mikan: " << MIKAN_RELEASE_VERSION_STRING << " MTL file" << std::endl;

		// Write out each mesh and corresponding material definition
		for (const GlTriangulatedMeshPtr triMesh : m_triangulatedMeshes)
		{
			// Make sure the material vertex definition has the needed attributes
			GlMaterialInstanceConstPtr materialInstance = triMesh->getMaterialInstance();
			GlMaterialConstPtr material = materialInstance->getMaterial();
			const GlVertexDefinition& vertexDefinition = material->getProgram()->getVertexDefinition();
			const size_t vertexSize = vertexDefinition.getVertexSize();
			const GlVertexAttribute* posAttrib = 
				vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::position);
			const GlVertexAttribute* normalAttrib = 
				vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::normal);
			const GlVertexAttribute* texelAttrib = 
				vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::texCoord);
			if (posAttrib == nullptr)
			{
				MIKAN_LOG_ERROR("GlRenderModelResource::saveToRenderModelFilePath()")
					<< "Material vertex definition missing needed attributes";
				continue;
			}

			objFile << "o " << triMesh->getName() << std::endl;

			const uint32_t vertexCount = triMesh->getVertexCount();
			const uint8_t* vertexData = triMesh->getVertexData();

			// Write out the vertices
			{
				const uint8_t* posData = vertexData + posAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec3& pos = *(const glm::vec3*)vertexData;

					objFile << "v " << pos.x << " " << pos.y << " " << pos.z << std::endl;
					posData+= vertexSize;
				}
			}

			// Write out the normals, if available
			if (normalAttrib)
			{
				const uint8_t* normalData = vertexData + normalAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec3& normal = *(const glm::vec3*)normalData;

					objFile << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
					normalData+= vertexSize;
				}
			}

			// Write out the texels, if available
			if (texelAttrib)
			{
				const uint8_t* texelData = vertexData + texelAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec2& texel = *(const glm::vec2*)texelData;

					objFile << "vt " << texel.x << " " << texel.y << std::endl;
					texelData+= vertexSize;
				}
			}

			// Write out the elements (usually triangles)
			{
				objFile << "usemtl " << material->getName() << std::endl;

				const uint8_t* indexData= triMesh->getIndexData();
				const size_t indexPerElements= triMesh->getIndexPerElementCount();
				const size_t indexSize= triMesh->getIndexSize();

				for (uint32_t i = 0; i < triMesh->getElementCount(); i++)
				{
					objFile << "f";
					for (size_t j = 0; j < indexPerElements; j++)
					{
						uint32_t index = 0;
						if (indexSize == sizeof(uint16_t))
						{
							index = (uint32_t)(*(const uint16_t*)indexData);
						}
						else
						{
							index = *(const uint32_t*)indexData;
						}

						uint32_t oneBasedIndex = index + 1;
						if (texelAttrib && normalAttrib)
						{
							objFile << " " << oneBasedIndex << "/" << oneBasedIndex << "/" << oneBasedIndex;
						}
						else if (normalAttrib)
						{
							objFile << " " << oneBasedIndex << "//" << oneBasedIndex;
						}
						else if (texelAttrib)
						{
							objFile << " " << oneBasedIndex << "/" << oneBasedIndex;
						}
						else
						{
							objFile << " " << oneBasedIndex;
						}

						indexData += indexSize;
					}
					objFile << std::endl;
				}
			}

			// Write out the material
			{
				mtlFile << std::endl;
				mtlFile << "newmtl " << material->getName() << std::endl;

				float Ns;
				bool hasNs = materialInstance->getFloatBySemantic(eUniformSemantic::specularHighlights, Ns);
				if (hasNs)
				{
					mtlFile << "Ns " << Ns << std::endl;
				}

				glm::vec4 Ka;
				bool hasKa = materialInstance->getVec4BySemantic(eUniformSemantic::ambientColorRGBA, Ka);
				if (hasKa)
				{
					mtlFile << "Ka " << Ka.r << " " << Ka.g << " " << Ka.b << std::endl;
				}

				glm::vec4 Kd;
				bool hasKd = materialInstance->getVec4BySemantic(eUniformSemantic::diffuseColorRGBA, Kd);
				if (hasKd)
				{
					mtlFile << "Kd " << Kd.r << " " << Kd.g << " " << Kd.b << std::endl;
				}

				glm::vec4 Ks;
				bool hasKs= materialInstance->getVec4BySemantic(eUniformSemantic::specularColorRGBA, Ks);
				if (hasKs)
				{
					mtlFile << "Ks " << Ks.r << " " << Ks.g << " " << Ks.b << std::endl;
				}

				float Ni;
				bool hasNi= materialInstance->getFloatBySemantic(eUniformSemantic::opticalDensity, Ni);
				if (hasNi)
				{
					mtlFile << "Ni " << Ni << std::endl;
				}

				float d;
				float hasD= materialInstance->getFloatBySemantic(eUniformSemantic::dissolve, d);
				if (hasD)
				{
					mtlFile << "d " << d << std::endl;
				}

				//illum 2: a diffuse and specular illumination model using Lambertian shading 
				// and Blinn's interpretation of Phong's specular illumination model, 
				// taking into account Ka, Kd, Ks, and the intensity and position of 
				// each light source and the angle at which it strikes the surface.
				if (hasKa && hasKd && hasKs)
				{
					mtlFile << "illum 2" << std::endl;
				}
				//illum 1: a diffuse illumination model using Lambertian shading, 
				//taking into account Ka, Kd, the intensity and position of each light source 
				//and the angle at which it strikes the surface.
				else if (hasKa && hasKd)
				{
					mtlFile << "illum 1" << std::endl;
				}
				//illum 0: a constant color illumination model, using the Kd for the material
				else
				{
					mtlFile << "illum 0" << std::endl;
				}

				// Write out the textures, if available
				std::string relativeTexturePath;
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::ambientTexture, relativeTexturePath))
				{
					mtlFile << "map_Ka " << relativeTexturePath << std::endl;
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::diffuseTexture, relativeTexturePath))
				{
					mtlFile << "map_Kd " << relativeTexturePath << std::endl;
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::specularTexture, relativeTexturePath))
				{
					mtlFile << "map_Ks " << relativeTexturePath << std::endl;
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::specularHightlightTexture, relativeTexturePath))
				{
					mtlFile << "map_Ns " << relativeTexturePath << std::endl;
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::alphaTexture, relativeTexturePath))
				{
					mtlFile << "map_d " << relativeTexturePath << std::endl;
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::bumpTexture, relativeTexturePath))
				{
					mtlFile << "map_bump " << relativeTexturePath << std::endl;
				}
			}
		}

		objFile.close();
		mtlFile.close();
	}

	return true;
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
			if (!textureRelativePath.empty())
			{
				std::filesystem::path texturePath = modelFilePath.parent_path() / textureRelativePath;
				
				texture = textureCache->loadTexturePath(texturePath);
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

	bool writeMaterialInstanceTextureToFile(
		GlMaterialInstanceConstPtr materialInstance,
		const std::filesystem::path& mtlPath,
		const eUniformSemantic semantic,
		std::string& outRelativeTexturePath)
	{
		GlProgramPtr program= materialInstance->getMaterial()->getProgram();

		std::string uniformName;
		GlTexturePtr texture;
		if (program->getFirstUniformNameOfSemantic(semantic, uniformName) &&
			materialInstance->getTextureBySemantic(semantic, texture) &&
			texture != nullptr)
		{
			const std::string modelFileStem= mtlPath.stem().string();
			const std::string textureFileName= modelFileStem + std::string("_") + uniformName + std::string(".png");
			const std::filesystem::path textureFullPath= mtlPath.parent_path() / textureFileName;
			const std::string texturePathString= textureFullPath.string(); 

			if (SdlUtility::saveTextureToPNG(texture.get(), texturePathString.c_str()))
			{
				outRelativeTexturePath= textureFileName;
				return true;
			}
			else
			{
				MIKAN_LOG_ERROR("ObjUtils::writeMaterialInstanceTextureToFile()")
					<< "Error writing out texture: " << texturePathString;
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
			textureCache, materialInstance, modelFilePath, objMaterial.map_Ka, eUniformSemantic::ambientTexture);
		// Diffuse Texture Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, modelFilePath, objMaterial.map_Kd, eUniformSemantic::diffuseTexture);
		// Specular Texture Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, modelFilePath, objMaterial.map_Ks, eUniformSemantic::specularTexture);
		// Specular Hightlight Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, modelFilePath, objMaterial.map_Ns, eUniformSemantic::specularHightlightTexture);
		// Alpha Texture Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, modelFilePath, objMaterial.map_d, eUniformSemantic::alphaTexture);
		// Bump Map
		addTextureToMaterialInstance(
			textureCache, materialInstance, modelFilePath, objMaterial.map_bump, eUniformSemantic::bumpTexture);

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
		const size_t vertexSize = vertexDefinition.getVertexSize();
		const size_t vertexBufferSize = vertexCount * vertexDefinition.getVertexSize();
		uint8_t* vertexData = new uint8_t[vertexBufferSize];
		memset(vertexData, 0, vertexBufferSize);

		// Copy in vertex data requested by 
		for (const GlVertexAttribute& attrib : vertexDefinition.getAttributes())
		{
			uint8_t* writePtr = &vertexData[attrib.getOffset()];

			switch (attrib.getSemantic())
			{
			case eVertexSemantic::position:
				{
					assert(attrib.getDataType() == eVertexDataType::datatype_vec3f);
					for (const objl::Vertex& vertex : objMesh.Vertices)
					{
						memcpy(writePtr, &vertex.Position, sizeof(float)*3);
						writePtr+= vertexSize;
					}
				} break;
			case eVertexSemantic::normal:
				{
					assert(attrib.getDataType() == eVertexDataType::datatype_vec3f);
					for (const objl::Vertex& vertex : objMesh.Vertices)
					{
						memcpy(writePtr, &vertex.Normal, sizeof(float) * 3);
						writePtr += vertexSize;
					}
				} break;
			case eVertexSemantic::texCoord:
				{
					assert(attrib.getDataType() == eVertexDataType::datatype_vec2f);
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