#include "GlCamera.h"
#include "GlMaterialInstance.h"
#include "GlMaterial.h"
#include "GlProgram.h"
#include "GlTexture.h"

GlMaterialInstance::GlMaterialInstance()
	: m_parentMaterial(nullptr)
{
}

GlMaterialInstance::GlMaterialInstance(GlMaterialConstPtr material)
	: m_parentMaterial(material)
{
}

GlMaterialInstance::GlMaterialInstance(GlMaterialInstanceConstPtr materialInstance)
	: m_parentMaterial(materialInstance->getMaterial())
	, m_floatSources(materialInstance->m_floatSources)
	, m_float2Sources(materialInstance->m_float2Sources)
	, m_float3Sources(materialInstance->m_float3Sources)
	, m_float4Sources(materialInstance->m_float4Sources)
	, m_mat4Sources(materialInstance->m_mat4Sources)
	, m_textureSources(materialInstance->m_textureSources)
{
}

bool GlMaterialInstance::setFloatBySemantic(eUniformSemantic semantic, float value)
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setFloatByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterialInstance::getFloatBySemantic(eUniformSemantic semantic, float& outValue) const
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getFloatByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterialInstance::setFloatByUniformName(const std::string uniformName, float value)
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float)
	{
		m_floatSources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterialInstance::getFloatByUniformName(const std::string uniformName, float& outValue) const
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float)
	{
		if (!m_floatSources.tryGetValue(uniformName, outValue))
			return m_parentMaterial->getFloatByUniformName(uniformName, outValue);
		else
			return true;

	}

	return false;
}

bool GlMaterialInstance::setVec2BySemantic(eUniformSemantic semantic, const glm::vec2& value)
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec2ByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterialInstance::getVec2BySemantic(eUniformSemantic semantic, glm::vec2& outValue) const
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec2ByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterialInstance::setVec2ByUniformName(const std::string uniformName, const glm::vec2& value)
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float2)
	{
		m_float2Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterialInstance::getVec2ByUniformName(const std::string uniformName, glm::vec2& outValue) const
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float2)
	{
		if (!m_float2Sources.tryGetValue(uniformName, outValue))
			return m_parentMaterial->getVec2ByUniformName(uniformName, outValue);
		else
			return true;
	}

	return false;
}

bool GlMaterialInstance::setVec3BySemantic(eUniformSemantic semantic, const glm::vec3& value)
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec3ByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterialInstance::getVec3BySemantic(eUniformSemantic semantic, glm::vec3& outValue) const
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec3ByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterialInstance::setVec3ByUniformName(const std::string uniformName, const glm::vec3& value)
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float3)
	{
		m_float3Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterialInstance::getVec3ByUniformName(const std::string uniformName, glm::vec3& outValue) const
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float3)
	{
		if (!m_float3Sources.tryGetValue(uniformName, outValue))
			return m_parentMaterial->getVec3ByUniformName(uniformName, outValue);
		else
			return true;
	}

	return false;
}

bool GlMaterialInstance::setVec4BySemantic(eUniformSemantic semantic, const glm::vec4& value)
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec4ByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterialInstance::getVec4BySemantic(eUniformSemantic semantic, glm::vec4& outValue) const
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec4ByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterialInstance::setVec4ByUniformName(const std::string uniformName, const glm::vec4& value)
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float4)
	{
		m_float4Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterialInstance::getVec4ByUniformName(const std::string uniformName, glm::vec4& outValue) const
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float4)
	{
		if (!m_float4Sources.tryGetValue(uniformName, outValue))
			return m_parentMaterial->getVec4ByUniformName(uniformName, outValue);
		else
			return true;
	}

	return false;
}

bool GlMaterialInstance::setMat4BySemantic(eUniformSemantic semantic, const glm::mat4& value)
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setMat4ByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterialInstance::getMat4BySemantic(eUniformSemantic semantic, glm::mat4& outValue) const
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getMat4ByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterialInstance::setMat4ByUniformName(const std::string uniformName, const glm::mat4& value)
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_mat4)
	{
		m_mat4Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterialInstance::getMat4ByUniformName(const std::string uniformName, glm::mat4& outValue) const
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_mat4)
	{
		if (!m_mat4Sources.tryGetValue(uniformName, outValue))
			return m_parentMaterial->getMat4ByUniformName(uniformName, outValue);
		else
			return true;
	}

	return false;
}

bool GlMaterialInstance::setTextureBySemantic(eUniformSemantic semantic, GlTexturePtr texture)
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setTextureByUniformName(uniformName, texture);
	}

	return false;
}

bool GlMaterialInstance::getTextureBySemantic(eUniformSemantic semantic, GlTexturePtr& outTexture) const
{
	std::string uniformName;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getTextureByUniformName(uniformName, outTexture);
	}

	return false;
}

bool GlMaterialInstance::setTextureByUniformName(const std::string uniformName, GlTexturePtr texture)
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_texture)
	{
		m_textureSources.setValue(uniformName, texture);
		return true;
	}

	return false;
}

bool GlMaterialInstance::getTextureByUniformName(const std::string uniformName, GlTexturePtr& outTexture) const
{
	eUniformDataType datatype;
	if (m_parentMaterial != nullptr &&
		m_parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_texture)
	{
		if (!m_textureSources.tryGetValue(uniformName, outTexture))
			return m_parentMaterial->getTextureByUniformName(uniformName, outTexture);
		else
			return true;
	}

	return false;
}

static void mark_uniform_as_bound(const std::string& uniformName, UniformNameSet& uniformSet)
{
	auto it= uniformSet.find(uniformName);
	if (it != uniformSet.end())
	{
		uniformSet.erase(it);
	}
}

GlScopedMaterialInstanceBinding GlMaterialInstance::bindMaterialInstance(
	const GlScopedMaterialBinding& materialBinding,
	BindUniformCallback callback) const
{
	bool bMaterialInstanceFailure= false;

	if (m_parentMaterial != nullptr && 
		materialBinding.getBoundMaterial() == m_parentMaterial)
	{
		UniformNameSet unboundUniforms= materialBinding.getUnboundUniforms();
		GlProgramPtr program= m_parentMaterial->getProgram();


		// Auto-apply callback specific uniform bindings first
		if (callback)
		{
			for (auto it = program->getUniformBegin(); it != program->getUniformEnd(); it++)
			{
				const std::string& uniformName = it->first;
				eUniformSemantic uniformSemantic = it->second.semantic;
				eUniformDataType uniformDataType = GlProgram::getUniformSemanticDataType(uniformSemantic);

				eUniformBindResult bindResult = callback(program, uniformDataType, uniformSemantic, uniformName);
				if (bindResult == eUniformBindResult::bound)
				{
					mark_uniform_as_bound(uniformName, unboundUniforms);
				}
				else if (bindResult == eUniformBindResult::error)
				{
					bMaterialInstanceFailure = true;
				}
			}
		}

		// Apply float overrides
		for (auto it = m_floatSources.getMap().begin(); it != m_floatSources.getMap().end(); ++it)
		{
			if (program->setFloatUniform(it->first, it->second))
			{
				mark_uniform_as_bound(it->first, unboundUniforms);
			}
			else
			{
				bMaterialInstanceFailure= true;
			}
		}

		// Apply vec2 overrides
		for (auto it = m_float2Sources.getMap().begin(); it != m_float2Sources.getMap().end(); ++it)
		{
			if (program->setVector2Uniform(it->first, it->second))
			{
				mark_uniform_as_bound(it->first, unboundUniforms);
			}
			else
			{
				bMaterialInstanceFailure = true;
			}
		}

		// Apply vec3 overrides
		for (auto it = m_float3Sources.getMap().begin(); it != m_float3Sources.getMap().end(); ++it)
		{
			if (program->setVector3Uniform(it->first, it->second))
			{
				mark_uniform_as_bound(it->first, unboundUniforms);
			}
			else
			{
				bMaterialInstanceFailure = true;
			}
		}

		// Apply vec4 overrides
		for (auto it = m_float4Sources.getMap().begin(); it != m_float4Sources.getMap().end(); ++it)
		{
			if (program->setVector4Uniform(it->first, it->second))
			{
				mark_uniform_as_bound(it->first, unboundUniforms);
			}
			else
			{
				bMaterialInstanceFailure = true;
			}
		}

		// Apply mat4 overrides
		for (auto it = m_mat4Sources.getMap().begin(); it != m_mat4Sources.getMap().end(); ++it)
		{
			if (program->setMatrix4x4Uniform(it->first, it->second))
			{
				mark_uniform_as_bound(it->first, unboundUniforms);
			}
			else
			{
				bMaterialInstanceFailure = true;
			}
		}

		// Apply texture overrides
		for (auto it = m_textureSources.getMap().begin(); it != m_textureSources.getMap().end(); ++it)
		{
			const std::string& uniformName= it->first;
			GlTexturePtr texture= it->second;
			int textureUnit;

			if (texture &&
				program->getUniformTextureUnit(uniformName, textureUnit) &&
				program->setTextureUniform(uniformName) &&
				texture->bindTexture(textureUnit))
			{
				mark_uniform_as_bound(it->first, unboundUniforms);
			}
			else
			{
				bMaterialInstanceFailure = true;
			}
		}

		// Make sure all uniforms were bound
		if (unboundUniforms.size() > 0)
		{
			// Failed to bind all material parameters
			bMaterialInstanceFailure= true;
		}
	}
	else
	{
		bMaterialInstanceFailure= true;
	}

	return GlScopedMaterialInstanceBinding(shared_from_this(), bMaterialInstanceFailure);
}

void GlMaterialInstance::unbindMaterialInstance() const
{
	if (m_parentMaterial == nullptr)
		return;

	// Unbind all textures
	for (auto it = m_textureSources.getMap().begin(); it != m_textureSources.getMap().end(); ++it)
	{
		const std::string& uniformName = it->first;
		GlTexturePtr texture = it->second;

		int textureUnit;
		if (m_parentMaterial->getProgram()->getUniformTextureUnit(uniformName, textureUnit))
		{
			it->second->clearTexture(textureUnit);
		}
	}
}

// -- GlScopedMaterialInstanceBinding ------
GlScopedMaterialInstanceBinding::~GlScopedMaterialInstanceBinding()
{
	if (m_boundMaterialInstance != nullptr)
	{
		m_boundMaterialInstance->unbindMaterialInstance();
	}
}