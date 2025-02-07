#include "GlMaterial.h"
#include "MikanShaderCache.h"
#include "GlProgram.h"
#include "GlProgramConfig.h"
#include "MaterialAssetReference.h"
#include "Logger.h"

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

		GlProgramConfig programConfig;
		if (programConfig.load(shaderFilePath))
		{
			GlProgramCode programCode;
			if (programCode.loadFromConfigData(programConfig))
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

GlMaterialPtr MikanShaderCache::registerMaterial(const GlProgramCode& code)
{
	return m_shaderCache->registerMaterial(code);
}

GlMaterialConstPtr MikanShaderCache::getMaterialByName(const std::string& name)
{
	return m_shaderCache->getMaterialByName(name);
}

GlProgramPtr MikanShaderCache::fetchCompiledGlProgram(
	const GlProgramCode* code)
{
	return m_shaderCache->fetchCompiledGlProgram(code);
}