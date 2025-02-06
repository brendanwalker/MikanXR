#include "ObjModelExporter.h"
#include "Colors.h"
#include "IMkWindow.h"
#include "GlRenderModelResource.h"
#include "GlModelResourceManager.h"
#include "GlMaterialInstance.h"
#include "GlMaterial.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlTextureCache.h"
#include "GlTriangulatedMesh.h"
#include "GlWireframeMesh.h"
#include "Logger.h"
#include "SdlUtility.h"
#include "StringUtils.h"
#include "Version.h"

#include <glm/ext/vector_float4.hpp>

#include <filesystem>

#include "stdio.h"

class BufferedFileWriter
{
public:
	BufferedFileWriter(const std::filesystem::path& filePath, size_t bufferCapacity)
		: m_bufferCapacity(bufferCapacity)
		, m_buffer(new char[bufferCapacity])
		, m_file(fopen(filePath.string().c_str(), "wb"))
	{
	}

	~BufferedFileWriter()
	{
		if (m_file)
		{
			flushBuffer();
			fclose(m_file);
		}

		delete[] m_buffer;
	}

	inline bool isOpen() const
	{
		return m_file != nullptr;
	}

	void writeString(const std::string& str)
	{
		writeChars(str.c_str(), str.size());
	}

	void writeChars(const char* szLine, size_t lineBytes)
	{
		if (m_bufferSize + lineBytes >= m_bufferCapacity)
		{
			flushBuffer();
		}

		memcpy(m_buffer + m_bufferSize, szLine, lineBytes);
		m_bufferSize += lineBytes;
	}

private:
	const size_t m_bufferCapacity = 1024;
	char* m_buffer= nullptr;
	size_t m_bufferSize = 0;
	FILE* m_file= nullptr;

	void flushBuffer()
	{
		if (m_file != nullptr && m_bufferSize > 0)
		{
			fwrite(m_buffer, m_bufferSize, 1, m_file);
			m_bufferSize = 0;
		}
	}
};

namespace ObjUtils
{
	bool writeMaterialInstanceTextureToFile(
		GlMaterialInstanceConstPtr materialInstance,
		const std::filesystem::path& mtlPath,
		const eUniformSemantic semantic,
		std::string& outRelativeTexturePath)
	{
		GlProgramPtr program = materialInstance->getMaterial()->getProgram();

		std::string uniformName;
		GlTexturePtr texture;
		if (program->getFirstUniformNameOfSemantic(semantic, uniformName) &&
			materialInstance->getTextureBySemantic(semantic, texture) &&
			texture != nullptr)
		{
			const std::string modelFileStem = mtlPath.stem().string();
			const std::string textureFileName = modelFileStem + std::string("_") + uniformName + std::string(".png");
			const std::filesystem::path textureFullPath = mtlPath.parent_path() / textureFileName;
			const std::string texturePathString = textureFullPath.string();

			if (SdlUtility::saveTextureToPNG(texture.get(), texturePathString.c_str()))
			{
				outRelativeTexturePath = textureFileName;
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
};
bool ObjModelExporter::exportModelToFile(
	GlRenderModelResourcePtr modelResource,
	const std::filesystem::path& modelPath)
{
	if (modelPath.empty())
	{
		return false;
	}

	const std::string dir = modelPath.parent_path().string();
	const std::string stem = modelPath.stem().string();
	const std::filesystem::path mtlPath = modelPath.parent_path() / (stem + ".mtl");


	BufferedFileWriter objFile(modelPath, 1024 * 1024); // 1MB buffer for obj file
	BufferedFileWriter mtlFile(mtlPath, 1024); // 1KB buffer for mtl file
	if (objFile.isOpen() || mtlFile.isOpen())
	{
		char szLine[256];

		// Write out the obj header
		objFile.writeString(StringUtils::stringify("# Mikan: ", MIKAN_RELEASE_VERSION_STRING , "\n"));
		objFile.writeString(StringUtils::stringify("mtllib ", stem, ".mtl\n"));

		// Write out the mtl header
		mtlFile.writeString(StringUtils::stringify("# Mikan: ", MIKAN_RELEASE_VERSION_STRING, " MTL file\n"));

		// Write out each mesh and corresponding material definition
		for (int triMeshIndex= 0; triMeshIndex < modelResource->getTriangulatedMeshCount(); triMeshIndex++)
		{
			const GlTriangulatedMeshPtr triMesh = modelResource->getTriangulatedMesh(triMeshIndex);

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

			objFile.writeString(StringUtils::stringify("o ", triMesh->getName(), "\n"));

			const uint32_t vertexCount = triMesh->getVertexCount();
			const uint8_t* vertexData = triMesh->getVertexData();

			// Write out the vertices
			{
				const uint8_t* posData = vertexData + posAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec3& pos = *(const glm::vec3*)posData;

					size_t numChars= 
						StringUtils::formatString(
							szLine, sizeof(szLine), 
							"v %f %f %f\n", 
							pos.x, pos.y, pos.z);
					posData += vertexSize;

					objFile.writeChars(szLine, numChars);
				}
			}

			// Write out the normals, if available
			if (normalAttrib)
			{
				const uint8_t* normalData = vertexData + normalAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec3& normal = *(const glm::vec3*)normalData;

					size_t numChars =
						StringUtils::formatString(
							szLine, sizeof(szLine),
							"vn %f %f %f\n",
							normal.x, normal.y, normal.z);
					normalData += vertexSize;

					objFile.writeChars(szLine, numChars);
				}
			}

			// Write out the texels, if available
			if (texelAttrib)
			{
				const uint8_t* texelData = vertexData + texelAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec2& texel = *(const glm::vec2*)texelData;

					size_t numChars =
						StringUtils::formatString(
							szLine, sizeof(szLine),
							"vt %f %f\n",
							texel.x, texel.y);
					texelData += vertexSize;

					objFile.writeChars(szLine, numChars);
				}
			}

			// Write out the elements (usually triangles)
			{
				objFile.writeString(StringUtils::stringify("usemtl ", material->getName(), "\n"));

				const uint8_t* indexData = triMesh->getIndexData();
				const size_t indexPerElements = triMesh->getIndexPerElementCount();
				const size_t indexSize = triMesh->getIndexSize();

				for (uint32_t i = 0; i < triMesh->getElementCount(); i++)
				{
					objFile.writeChars("f", 1);

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
							size_t numChars = StringUtils::formatString(
								szLine, sizeof(szLine),
								" %d/%d/%d",
								oneBasedIndex, oneBasedIndex, oneBasedIndex);
							objFile.writeChars(szLine, numChars);
						}
						else if (normalAttrib)
						{
							size_t numChars = StringUtils::formatString(
								szLine, sizeof(szLine),
								" %d//%d",
								oneBasedIndex, oneBasedIndex);
							objFile.writeChars(szLine, numChars);
						}
						else if (texelAttrib)
						{
							size_t numChars = StringUtils::formatString(
								szLine, sizeof(szLine),
								" %d/%d",
								oneBasedIndex, oneBasedIndex);
							objFile.writeChars(szLine, numChars);
						}
						else
						{
							size_t numChars = StringUtils::formatString(
								szLine, sizeof(szLine),
								" %d",
								oneBasedIndex);
							objFile.writeChars(szLine, numChars);
						}

						indexData += indexSize;
					}

					objFile.writeChars("\n", 1);
				}
			}

			// Write out the material
			{
				mtlFile.writeChars("\n", 1);
				mtlFile.writeString(StringUtils::stringify("newmtl ", material->getName(), "\n"));

				float Ns;
				bool hasNs = materialInstance->getFloatBySemantic(eUniformSemantic::specularHighlights, Ns);
				if (hasNs)
				{
					mtlFile.writeString(StringUtils::stringify("Ns ", Ns, "\n"));
				}

				glm::vec3 Ka;
				bool hasKa = materialInstance->getVec3BySemantic(eUniformSemantic::ambientColorRGB, Ka);
				if (hasKa)
				{
					mtlFile.writeString(StringUtils::stringify("Ka ", Ka.r, " ", Ka.g, " ", Ka.b, "\n"));
				}

				glm::vec3 Kd = glm::vec3(1.f);
				bool hasKd = materialInstance->getVec3BySemantic(eUniformSemantic::diffuseColorRGB, Kd);
				mtlFile.writeString(StringUtils::stringify("Kd ", Kd.r, " ", Kd.g, " ", Kd.b, "\n"));

				glm::vec3 Ks;
				bool hasKs = materialInstance->getVec3BySemantic(eUniformSemantic::specularColorRGB, Ks);
				if (hasKs)
				{
					mtlFile.writeString(StringUtils::stringify("Ks ", Ks.r, " ", Ks.g, " ", Ks.b, "\n"));
				}

				float Ni;
				bool hasNi = materialInstance->getFloatBySemantic(eUniformSemantic::opticalDensity, Ni);
				if (hasNi)
				{
					mtlFile.writeString(StringUtils::stringify("Ni ", Ni, "\n"));
				}

				float d;
				float hasD = materialInstance->getFloatBySemantic(eUniformSemantic::dissolve, d);
				if (hasD)
				{
					mtlFile.writeString(StringUtils::stringify("d ", d, "\n"));
				}

				//illum 2: a diffuse and specular illumination model using Lambertian shading 
				// and Blinn's interpretation of Phong's specular illumination model, 
				// taking into account Ka, Kd, Ks, and the intensity and position of 
				// each light source and the angle at which it strikes the surface.
				if (hasKa && hasKd && hasKs)
				{
					mtlFile.writeString("illum 2\n");
				}
				//illum 1: a diffuse illumination model using Lambertian shading, 
				//taking into account Ka, Kd, the intensity and position of each light source 
				//and the angle at which it strikes the surface.
				else if (hasKa && hasKd)
				{
					mtlFile.writeString("illum 1\n");
				}
				//illum 0: a constant color illumination model, using the Kd for the material
				else
				{
					mtlFile.writeString("illum 0\n");
				}

				// Write out the textures, if available
				std::string relativeTexturePath;
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::ambientTexture, relativeTexturePath))
				{
					mtlFile.writeString(StringUtils::stringify("map_Ka ", relativeTexturePath, "\n"));
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::diffuseTexture, relativeTexturePath))
				{
					mtlFile.writeString(StringUtils::stringify("map_Kd ", relativeTexturePath, "\n"));
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::specularTexture, relativeTexturePath))
				{
					mtlFile.writeString(StringUtils::stringify("map_Ks ", relativeTexturePath, "\n"));
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::specularHightlightTexture, relativeTexturePath))
				{
					mtlFile.writeString(StringUtils::stringify("map_Ns ", relativeTexturePath, "\n"));
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::alphaTexture, relativeTexturePath))
				{
					mtlFile.writeString(StringUtils::stringify("map_d ", relativeTexturePath, "\n"));
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::bumpTexture, relativeTexturePath))
				{
					mtlFile.writeString(StringUtils::stringify("map_bump ", relativeTexturePath, "\n"));
				}
			}
		}
	}

	return true;
}