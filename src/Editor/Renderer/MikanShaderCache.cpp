#include "GlMaterial.h"
#include "MikanShaderCache.h"
#include "IMkShader.h"
#include "IMkShaderCode.h"
#include "MikanShaderConfig.h"
#include "MaterialAssetReference.h"
#include "Logger.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

MikanShaderCache::MikanShaderCache(IMkWindow* ownerWindow)
	: m_shaderCache(CreateMkShaderCache(ownerWindow))
{
}

bool MikanShaderCache::startup()
{
	return m_shaderCache->startup();
}

void MikanShaderCache::shutdown()
{
	m_shaderCache->shutdown();
}

GlMaterialPtr MikanShaderCache::loadMaterialAssetReference(MaterialAssetReferencePtr materialAssetRef)
{
	GlMaterialPtr material;

	if (materialAssetRef && materialAssetRef->isValid())
	{
		auto shaderFilePath = materialAssetRef->getAssetPath();

		MikanShaderConfig programConfig;
		if (programConfig.load(shaderFilePath))
		{
			IMkShaderCodeConstPtr programCode= loadShaderCodeFromConfigData(programConfig);

			if (programCode)
			{
				material = m_shaderCache->registerMaterial(programCode);
			}
			else
			{
				MIKAN_LOG_ERROR("GlShaderCache::loadMaterialAssetReference")
					<< "Failed material program code: " << shaderFilePath;
			}
		}
		else
		{
			MIKAN_LOG_ERROR("GlShaderCache::loadMaterialAssetReference")
				<< "Failed material config load: " << shaderFilePath;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("GlShaderCache::loadMaterialAssetReference") << "Invalid material asset ref";
	}

	return material;
}

IMkShaderCodeConstPtr MikanShaderCache::loadShaderCodeFromConfigData(const MikanShaderConfig& config)
{
	bool bSuccess = true;

	const std::filesystem::path& shaderConfigPath = config.getLoadedConfigPath();

	std::filesystem::path shaderFolderPath = shaderConfigPath;
	shaderFolderPath.remove_filename();

	std::string programName = shaderConfigPath.stem().string();

	std::filesystem::path vertexShaderFilePath;
	std::string vertexShaderCode;
	try
	{
		vertexShaderFilePath = shaderFolderPath;
		vertexShaderFilePath /= config.vertexShaderPath;

		std::ifstream t(vertexShaderFilePath.string());
		std::stringstream buffer;
		buffer << t.rdbuf();
		vertexShaderCode = buffer.str();
	}
	catch (const std::ifstream::failure& e)
	{
		MIKAN_LOG_ERROR("MikanShaderCache::loadFromConfigData")
			<< vertexShaderFilePath.string()
			<< " - unable to load vertex shader file!";
		return IMkShaderCodeConstPtr();
	}

	std::filesystem::path fragmentShaderFilePath;
	std::string fragmentShaderCode;
	try
	{
		fragmentShaderFilePath = shaderFolderPath;
		fragmentShaderFilePath /= config.fragmentShaderPath;

		std::ifstream t(fragmentShaderFilePath.string());
		std::stringstream buffer;
		buffer << t.rdbuf();
		fragmentShaderCode = buffer.str();
	}
	catch (const std::ifstream::failure& e)
	{
		MIKAN_LOG_ERROR("MikanShaderCache::loadFromConfigData")
			<< fragmentShaderFilePath.string()
			<< " - unable to load fragment shader file!";
		return IMkShaderCodeConstPtr();
	}

	IMkShaderCodePtr programCode = createIMkShaderCode(programName, vertexShaderCode, fragmentShaderCode);
	programCode->setVertexShaderFilePath(shaderConfigPath);
	programCode->setFragmentShaderFilePath(fragmentShaderFilePath);

	for (const GlVertexAttributeConfigPtr attribConfig : config.vertexAttributes)
	{
		if (attribConfig->dataType == eVertexDataType::INVALID ||
			attribConfig->semantic == eVertexSemantic::INVALID)
		{
			MIKAN_LOG_ERROR("IMkShaderCode::loadFromConfigData")
				<< "Invalid vertex attribute("
				<< attribConfig->name
				<< ") dataType="
				<< VertexConstantUtils::vertexDataTypeToString(attribConfig->dataType)
				<< ", semantic="
				<< VertexConstantUtils::vertexSemanticToString(attribConfig->semantic);
			return IMkShaderCodeConstPtr();
		}
		else
		{
			programCode->addVertexAttribute(attribConfig->name, attribConfig->dataType, attribConfig->semantic);
		}
	}

	for (const auto& [uniformName, semanticName] : config.uniformSemanticMap)
	{
		eUniformSemantic semantic = eUniformSemantic::INVALID;
		for (int enumIntValue = 0; enumIntValue < (int)eUniformSemantic::COUNT; ++enumIntValue)
		{
			if (k_UniformSemanticName[enumIntValue] == semanticName)
			{
				semantic = (eUniformSemantic)enumIntValue;
				break;
			}
		}

		if (semantic != eUniformSemantic::INVALID)
		{
			programCode->addUniform(uniformName, semantic);
		}
		else
		{
			MIKAN_LOG_ERROR("IMkShaderCode::loadFromConfigData")
				<< "Invalid semantic: "
				<< uniformName
				<< " -> "
				<< semanticName;
			return IMkShaderCodeConstPtr();
		}
	}

	return programCode;
}

GlMaterialPtr MikanShaderCache::registerMaterial(IMkShaderCodeConstPtr code)
{
	return m_shaderCache->registerMaterial(code);
}

GlMaterialConstPtr MikanShaderCache::getMaterialByName(const std::string& name)
{
	return m_shaderCache->getMaterialByName(name);
}

IMkShaderPtr MikanShaderCache::fetchCompiledIMkShader(
	IMkShaderCodeConstPtr code)
{
	return m_shaderCache->fetchCompiledIMkShader(code);
}