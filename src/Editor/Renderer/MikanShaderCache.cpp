#include "MkMaterial.h"
#include "MikanShaderCache.h"
#include "IMkShader.h"
#include "IMkShaderCode.h"
#include "MikanShaderConfig.h"
#include "MikanTextureCache.h"
#include "MaterialAssetReference.h"
#include "Logger.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

MikanShaderCache::MikanShaderCache(IMkGraphicsContext* graphicsContext)
	: m_shaderCache(createMkShaderCache(graphicsContext))
{
}

bool MikanShaderCache::startup() { return m_shaderCache->startup(); }

void MikanShaderCache::shutdown() { m_shaderCache->shutdown(); }

MkMaterialPtr MikanShaderCache::loadMaterialAssetReference(MaterialAssetReferencePtr materialAssetRef,
														   MikanShaderConfig* outConfig)
{
	MkMaterialPtr material;

	if (materialAssetRef && !materialAssetRef->isEmpty())
	{
		auto shaderFilePath= materialAssetRef->getInternalAssetPath();

		// Load straight into the caller's config when one was given, so the
		// caller sees the domain and source graph the material came from
		MikanShaderConfig localConfig;
		MikanShaderConfig& programConfig= outConfig ? *outConfig : localConfig;
		if (programConfig.load(shaderFilePath))
		{
			IMkShaderCodeConstPtr programCode= loadShaderCodeFromConfigData(programConfig);

			if (programCode)
			{
				material= m_shaderCache->registerMaterial(programCode);
				if (material)
				{
					applyMaterialDefaults(material, programConfig);
				}
			}
			else
			{
				MIKAN_LOG_ERROR("MikanShaderCache::loadMaterialAssetReference")
					<< "Failed material program code: " << shaderFilePath;
			}
		}
		else
		{
			MIKAN_LOG_ERROR("MikanShaderCache::loadMaterialAssetReference")
				<< "Failed material config load: " << shaderFilePath;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("MikanShaderCache::loadMaterialAssetReference") << "Invalid material asset ref";
	}

	return material;
}

void MikanShaderCache::applyMaterialDefaults(MkMaterialPtr material, const MikanShaderConfig& config)
{
	IMkShaderPtr program= material ? material->getProgram() : IMkShaderPtr();
	if (!program)
		return;

	// The setters type-check against the program, so a default for a uniform the
	// shader no longer declares (or declares with another type) is dropped
	for (const auto& [uniformName, values] : config.uniformFloatDefaults)
	{
		eUniformDataType dataType;
		if (!program->getUniformDataType(uniformName, dataType))
			continue;

		auto component= [&values](size_t index) { return index < values.size() ? values[index] : 0.f; };
		switch (dataType)
		{
		case eUniformDataType::datatype_float:
			material->setFloatByUniformName(uniformName, component(0));
			break;
		case eUniformDataType::datatype_float2:
			material->setVec2ByUniformName(uniformName, glm::vec2(component(0), component(1)));
			break;
		case eUniformDataType::datatype_float3:
			material->setVec3ByUniformName(uniformName, glm::vec3(component(0), component(1), component(2)));
			break;
		case eUniformDataType::datatype_float4:
			material->setVec4ByUniformName(uniformName,
										   glm::vec4(component(0), component(1), component(2), component(3)));
			break;
		default:
			break;
		}
	}

	if (m_textureCache == nullptr)
		return;

	for (const auto& [uniformName, texturePath] : config.uniformTextureDefaults)
	{
		IMkTexturePtr texture= m_textureCache->loadTexturePath(texturePath);
		if (texture)
		{
			material->setTextureByUniformName(uniformName, texture);
		}
		else
		{
			MIKAN_LOG_WARNING("MikanShaderCache::applyMaterialDefaults")
				<< "Default texture for " << uniformName << " failed to load: " << texturePath;
		}
	}
}

bool MikanShaderCache::reloadMaterialByPath(const std::filesystem::path& materialPath)
{
	// Materials are cached under their program name, which is the .mat stem
	const std::string programName= materialPath.stem().string();
	MkMaterialConstPtr cachedMaterial= m_shaderCache->getMaterialByName(programName);
	if (!cachedMaterial)
	{
		return false;
	}

	MikanShaderConfig programConfig;
	if (!programConfig.load(materialPath))
	{
		MIKAN_LOG_ERROR("MikanShaderCache::reloadMaterialByPath") << "Failed material config load: " << materialPath;
		return false;
	}

	IMkShaderCodeConstPtr programCode= loadShaderCodeFromConfigData(programConfig);
	if (!programCode)
	{
		MIKAN_LOG_ERROR("MikanShaderCache::reloadMaterialByPath") << "Failed material program code: " << materialPath;
		return false;
	}

	// The cache only hands out const materials; it owns the mutable one
	MkMaterialPtr material= std::const_pointer_cast<MkMaterial>(cachedMaterial);
	IMkShaderPtr oldProgram= material->getProgram();

	// Recompiles only when the code hash changed
	IMkShaderPtr program= m_shaderCache->fetchCompiledIMkShader(programCode);
	if (!program)
	{
		MIKAN_LOG_ERROR("MikanShaderCache::reloadMaterialByPath") << "Failed to compile material: " << programName;

		// The failed compile evicted the old program from the program cache, and the
		// material only holds it weakly. Rebuild it from its own code so the material
		// keeps drawing with the last program that compiled.
		if (oldProgram)
		{
			material->setProgram(m_shaderCache->fetchCompiledIMkShader(oldProgram->getProgramCode()));
		}

		return false;
	}

	material->setProgram(program);
	applyMaterialDefaults(material, programConfig);

	if (OnMaterialReloaded)
	{
		OnMaterialReloaded(material);
	}

	return true;
}

IMkShaderCodePtr MikanShaderCache::createShaderCode(const MikanShaderConfig& config, const std::string& programName,
													const std::string& vertexSource, const std::string& fragmentSource)
{
	IMkShaderCodePtr programCode= createIMkShaderCode(programName, vertexSource, fragmentSource);

	for (const GlVertexAttributeConfigPtr attribConfig : config.vertexAttributes)
	{
		if (attribConfig->dataType == eVertexDataType::INVALID || attribConfig->semantic == eVertexSemantic::INVALID)
		{
			MIKAN_LOG_ERROR("MikanShaderCache::createShaderCode")
				<< "Invalid vertex attribute(" << attribConfig->name
				<< ") dataType=" << VertexConstantUtils::vertexDataTypeToString(attribConfig->dataType)
				<< ", semantic=" << VertexConstantUtils::vertexSemanticToString(attribConfig->semantic);
			return IMkShaderCodePtr();
		}
		else
		{
			programCode->addVertexAttribute(attribConfig->name, attribConfig->dataType, attribConfig->semantic);
		}
	}

	for (const auto& [uniformName, semanticName] : config.uniformSemanticMap)
	{
		eUniformSemantic semantic= eUniformSemantic::INVALID;
		for (int enumIntValue= 0; enumIntValue < (int)eUniformSemantic::COUNT; ++enumIntValue)
		{
			const std::string enumSemanticName= getUniformSemanticName((eUniformSemantic)enumIntValue);

			if (enumSemanticName == semanticName)
			{
				semantic= (eUniformSemantic)enumIntValue;
				break;
			}
		}

		if (semantic != eUniformSemantic::INVALID)
		{
			programCode->addUniform(uniformName, semantic);
		}
		else
		{
			MIKAN_LOG_ERROR("MikanShaderCache::createShaderCode")
				<< "Invalid semantic: " << uniformName << " -> " << semanticName;
			return IMkShaderCodePtr();
		}
	}

	return programCode;
}

IMkShaderCodeConstPtr MikanShaderCache::loadShaderCodeFromConfigData(const MikanShaderConfig& config)
{
	const std::filesystem::path& shaderConfigPath= config.getLoadedConfigPath();

	std::filesystem::path shaderFolderPath= shaderConfigPath;
	shaderFolderPath.remove_filename();

	std::string programName= shaderConfigPath.stem().string();

	std::filesystem::path vertexShaderFilePath;
	std::string vertexShaderCode;
	try
	{
		vertexShaderFilePath= shaderFolderPath;
		vertexShaderFilePath/= config.vertexShaderPath;

		std::ifstream t(vertexShaderFilePath.string());
		std::stringstream buffer;
		buffer << t.rdbuf();
		vertexShaderCode= buffer.str();
	}
	catch (const std::ifstream::failure& e)
	{
		MIKAN_LOG_ERROR("MikanShaderCache::loadShaderCodeFromConfigData")
			<< vertexShaderFilePath.string() << " - unable to load vertex shader file!";
		return IMkShaderCodeConstPtr();
	}

	std::filesystem::path fragmentShaderFilePath;
	std::string fragmentShaderCode;
	try
	{
		fragmentShaderFilePath= shaderFolderPath;
		fragmentShaderFilePath/= config.fragmentShaderPath;

		std::ifstream t(fragmentShaderFilePath.string());
		std::stringstream buffer;
		buffer << t.rdbuf();
		fragmentShaderCode= buffer.str();
	}
	catch (const std::ifstream::failure& e)
	{
		MIKAN_LOG_ERROR("MikanShaderCache::loadShaderCodeFromConfigData")
			<< fragmentShaderFilePath.string() << " - unable to load fragment shader file!";
		return IMkShaderCodeConstPtr();
	}

	IMkShaderCodePtr programCode= createShaderCode(config, programName, vertexShaderCode, fragmentShaderCode);
	if (programCode)
	{
		programCode->setVertexShaderFilePath(vertexShaderFilePath);
		programCode->setFragmentShaderFilePath(fragmentShaderFilePath);
	}

	return programCode;
}

MkMaterialPtr MikanShaderCache::registerMaterial(IMkShaderCodeConstPtr code)
{
	return m_shaderCache->registerMaterial(code);
}

MkMaterialConstPtr MikanShaderCache::getMaterialByName(const std::string& name)
{
	return m_shaderCache->getMaterialByName(name);
}

IMkShaderPtr MikanShaderCache::fetchCompiledIMkShader(IMkShaderCodeConstPtr code)
{
	return m_shaderCache->fetchCompiledIMkShader(code);
}
