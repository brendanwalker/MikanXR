#include "ObjModelExporter.h"
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
#include "Logger.h"
#include "SdlUtility.h"
#include "Version.h"

#include <glm/ext/vector_float4.hpp>

#include <fstream>

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

	const std::size_t writeBufferSize = 1024 * 1024;
	char* writeBuffer = new char[writeBufferSize];

	std::ofstream objFile(modelPath);
	std::ofstream mtlFile(mtlPath);
	if (objFile.is_open() || mtlFile.is_open())
	{
		// Create a 1MB buffer 
		objFile.rdbuf()->pubsetbuf(writeBuffer, writeBufferSize);

		// Write out the obj header
		objFile << "# Mikan: " << MIKAN_RELEASE_VERSION_STRING << '\n';
		objFile << "mtllib " << stem << ".mtl" << '\n';

		// Write out the mtl header
		mtlFile << "# Mikan: " << MIKAN_RELEASE_VERSION_STRING << " MTL file" << '\n';

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

			objFile << "o " << triMesh->getName() << '\n';

			const uint32_t vertexCount = triMesh->getVertexCount();
			const uint8_t* vertexData = triMesh->getVertexData();

			// Write out the vertices
			{
				const uint8_t* posData = vertexData + posAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec3& pos = *(const glm::vec3*)posData;

					objFile << "v " << pos.x << " " << pos.y << " " << pos.z << '\n';
					posData += vertexSize;
				}
			}

			// Write out the normals, if available
			if (normalAttrib)
			{
				const uint8_t* normalData = vertexData + normalAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec3& normal = *(const glm::vec3*)normalData;

					objFile << "vn " << normal.x << " " << normal.y << " " << normal.z << '\n';
					normalData += vertexSize;
				}
			}

			// Write out the texels, if available
			if (texelAttrib)
			{
				const uint8_t* texelData = vertexData + texelAttrib->getOffset();
				for (uint32_t i = 0; i < vertexCount; i++)
				{
					const glm::vec2& texel = *(const glm::vec2*)texelData;

					objFile << "vt " << texel.x << " " << texel.y << '\n';
					texelData += vertexSize;
				}
			}

			// Write out the elements (usually triangles)
			{
				objFile << "usemtl " << material->getName() << '\n';

				const uint8_t* indexData = triMesh->getIndexData();
				const size_t indexPerElements = triMesh->getIndexPerElementCount();
				const size_t indexSize = triMesh->getIndexSize();

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
					objFile << '\n';
				}
			}

			// Write out the material
			{
				mtlFile << '\n';
				mtlFile << "newmtl " << material->getName() << '\n';

				float Ns;
				bool hasNs = materialInstance->getFloatBySemantic(eUniformSemantic::specularHighlights, Ns);
				if (hasNs)
				{
					mtlFile << "Ns " << Ns << '\n';
				}

				glm::vec3 Ka;
				bool hasKa = materialInstance->getVec3BySemantic(eUniformSemantic::ambientColorRGB, Ka);
				if (hasKa)
				{
					mtlFile << "Ka " << Ka.r << " " << Ka.g << " " << Ka.b << '\n';
				}

				glm::vec3 Kd = glm::vec3(1.f);
				bool hasKd = materialInstance->getVec3BySemantic(eUniformSemantic::diffuseColorRGB, Kd);
				mtlFile << "Kd " << Kd.r << " " << Kd.g << " " << Kd.b << '\n';

				glm::vec3 Ks;
				bool hasKs = materialInstance->getVec3BySemantic(eUniformSemantic::specularColorRGB, Ks);
				if (hasKs)
				{
					mtlFile << "Ks " << Ks.r << " " << Ks.g << " " << Ks.b << '\n';
				}

				float Ni;
				bool hasNi = materialInstance->getFloatBySemantic(eUniformSemantic::opticalDensity, Ni);
				if (hasNi)
				{
					mtlFile << "Ni " << Ni << '\n';
				}

				float d;
				float hasD = materialInstance->getFloatBySemantic(eUniformSemantic::dissolve, d);
				if (hasD)
				{
					mtlFile << "d " << d << '\n';
				}

				//illum 2: a diffuse and specular illumination model using Lambertian shading 
				// and Blinn's interpretation of Phong's specular illumination model, 
				// taking into account Ka, Kd, Ks, and the intensity and position of 
				// each light source and the angle at which it strikes the surface.
				if (hasKa && hasKd && hasKs)
				{
					mtlFile << "illum 2" << '\n';
				}
				//illum 1: a diffuse illumination model using Lambertian shading, 
				//taking into account Ka, Kd, the intensity and position of each light source 
				//and the angle at which it strikes the surface.
				else if (hasKa && hasKd)
				{
					mtlFile << "illum 1" << '\n';
				}
				//illum 0: a constant color illumination model, using the Kd for the material
				else
				{
					mtlFile << "illum 0" << '\n';
				}

				// Write out the textures, if available
				std::string relativeTexturePath;
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::ambientTexture, relativeTexturePath))
				{
					mtlFile << "map_Ka " << relativeTexturePath << '\n';
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::diffuseTexture, relativeTexturePath))
				{
					mtlFile << "map_Kd " << relativeTexturePath << '\n';
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::specularTexture, relativeTexturePath))
				{
					mtlFile << "map_Ks " << relativeTexturePath << '\n';
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::specularHightlightTexture, relativeTexturePath))
				{
					mtlFile << "map_Ns " << relativeTexturePath << '\n';
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::alphaTexture, relativeTexturePath))
				{
					mtlFile << "map_d " << relativeTexturePath << '\n';
				}
				if (ObjUtils::writeMaterialInstanceTextureToFile(
					materialInstance, mtlPath, eUniformSemantic::bumpTexture, relativeTexturePath))
				{
					mtlFile << "map_bump " << relativeTexturePath << '\n';
				}
			}
		}

		objFile.close();
		mtlFile.close();
	}

	delete[] writeBuffer;
	writeBuffer = nullptr;

	return true;
}