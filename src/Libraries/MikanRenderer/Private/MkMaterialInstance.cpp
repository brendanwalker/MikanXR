#include "MkMaterialInstance.h"
#include "MkMaterial.h"
#include "IMkShader.h"
#include "IMkTexture.h"

struct MkMaterialInstanceImpl
{
	MkMaterialConstPtr parentMaterial;

	// Material Override Parameters
	NamedValueTable<float> floatSources;
	NamedValueTable<glm::vec2> float2Sources;
	NamedValueTable<glm::vec3> float3Sources;
	NamedValueTable<glm::vec4> float4Sources;
	NamedValueTable<glm::mat4> mat4Sources;
	NamedValueTable<IMkTexturePtr> textureSources;
};


MkMaterialInstance::MkMaterialInstance()
	: m_impl(new MkMaterialInstanceImpl)
{
	m_impl->parentMaterial= nullptr;
}

MkMaterialInstance::MkMaterialInstance(MkMaterialConstPtr material)
	: m_impl(new MkMaterialInstanceImpl)
{
	m_impl->parentMaterial = material;
}

MkMaterialInstance::MkMaterialInstance(MkMaterialInstanceConstPtr materialInstance)
	: m_impl(new MkMaterialInstanceImpl)
{
	m_impl->parentMaterial= materialInstance->getMaterial();
	m_impl->floatSources= materialInstance->m_impl->floatSources;
	m_impl->float2Sources= materialInstance->m_impl->float2Sources;
	m_impl->float3Sources= materialInstance->m_impl->float3Sources;
	m_impl->float4Sources= materialInstance->m_impl->float4Sources;
	m_impl->mat4Sources= materialInstance->m_impl->mat4Sources;
	m_impl->textureSources= materialInstance->m_impl->textureSources;
}

MkMaterialConstPtr MkMaterialInstance::getMaterial() const
{
	return m_impl->parentMaterial;
}

bool MkMaterialInstance::setFloatBySemantic(eUniformSemantic semantic, float value)
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setFloatByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterialInstance::getFloatBySemantic(eUniformSemantic semantic, float& outValue) const
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getFloatByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterialInstance::setFloatByUniformName(const std::string uniformName, float value)
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float)
	{
		m_impl->floatSources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterialInstance::getFloatByUniformName(const std::string uniformName, float& outValue) const
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float)
	{
		if (!m_impl->floatSources.tryGetValue(uniformName, outValue))
			return m_impl->parentMaterial->getFloatByUniformName(uniformName, outValue);
		else
			return true;

	}

	return false;
}

bool MkMaterialInstance::setVec2BySemantic(eUniformSemantic semantic, const glm::vec2& value)
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec2ByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterialInstance::getVec2BySemantic(eUniformSemantic semantic, glm::vec2& outValue) const
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec2ByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterialInstance::setVec2ByUniformName(const std::string uniformName, const glm::vec2& value)
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float2)
	{
		m_impl->float2Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterialInstance::getVec2ByUniformName(const std::string uniformName, glm::vec2& outValue) const
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float2)
	{
		if (!m_impl->float2Sources.tryGetValue(uniformName, outValue))
			return m_impl->parentMaterial->getVec2ByUniformName(uniformName, outValue);
		else
			return true;
	}

	return false;
}

bool MkMaterialInstance::setVec3BySemantic(eUniformSemantic semantic, const glm::vec3& value)
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec3ByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterialInstance::getVec3BySemantic(eUniformSemantic semantic, glm::vec3& outValue) const
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec3ByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterialInstance::setVec3ByUniformName(const std::string uniformName, const glm::vec3& value)
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float3)
	{
		m_impl->float3Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterialInstance::getVec3ByUniformName(const std::string uniformName, glm::vec3& outValue) const
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float3)
	{
		if (!m_impl->float3Sources.tryGetValue(uniformName, outValue))
			return m_impl->parentMaterial->getVec3ByUniformName(uniformName, outValue);
		else
			return true;
	}

	return false;
}

bool MkMaterialInstance::setVec4BySemantic(eUniformSemantic semantic, const glm::vec4& value)
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec4ByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterialInstance::getVec4BySemantic(eUniformSemantic semantic, glm::vec4& outValue) const
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec4ByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterialInstance::setVec4ByUniformName(const std::string uniformName, const glm::vec4& value)
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float4)
	{
		m_impl->float4Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterialInstance::getVec4ByUniformName(const std::string uniformName, glm::vec4& outValue) const
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float4)
	{
		if (!m_impl->float4Sources.tryGetValue(uniformName, outValue))
			return m_impl->parentMaterial->getVec4ByUniformName(uniformName, outValue);
		else
			return true;
	}

	return false;
}

bool MkMaterialInstance::setMat4BySemantic(eUniformSemantic semantic, const glm::mat4& value)
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setMat4ByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterialInstance::getMat4BySemantic(eUniformSemantic semantic, glm::mat4& outValue) const
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getMat4ByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterialInstance::setMat4ByUniformName(const std::string uniformName, const glm::mat4& value)
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_mat4)
	{
		m_impl->mat4Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterialInstance::getMat4ByUniformName(const std::string uniformName, glm::mat4& outValue) const
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_mat4)
	{
		if (!m_impl->mat4Sources.tryGetValue(uniformName, outValue))
			return m_impl->parentMaterial->getMat4ByUniformName(uniformName, outValue);
		else
			return true;
	}

	return false;
}

bool MkMaterialInstance::setTextureBySemantic(eUniformSemantic semantic, IMkTexturePtr texture)
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setTextureByUniformName(uniformName, texture);
	}

	return false;
}

bool MkMaterialInstance::getTextureBySemantic(eUniformSemantic semantic, IMkTexturePtr& outTexture) const
{
	std::string uniformName;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getTextureByUniformName(uniformName, outTexture);
	}

	return false;
}

bool MkMaterialInstance::setTextureByUniformName(const std::string uniformName, IMkTexturePtr texture)
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_texture)
	{
		m_impl->textureSources.setValue(uniformName, texture);
		return true;
	}

	return false;
}

bool MkMaterialInstance::getTextureByUniformName(const std::string uniformName, IMkTexturePtr& outTexture) const
{
	eUniformDataType datatype;
	if (m_impl->parentMaterial != nullptr &&
		m_impl->parentMaterial->getProgram()->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_texture)
	{
		if (!m_impl->textureSources.tryGetValue(uniformName, outTexture))
			return m_impl->parentMaterial->getTextureByUniformName(uniformName, outTexture);
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

MkScopedMaterialInstanceBinding MkMaterialInstance::bindMaterialInstance(
	const MkScopedMaterialBinding& materialBinding,
	BindUniformCallback callback) const
{
	bool bMaterialInstanceFailure= false;
	UniformNameSet unboundUniforms = materialBinding.getUnboundUniforms();

	if (m_impl->parentMaterial != nullptr && 
		materialBinding.getBoundMaterial() == m_impl->parentMaterial)
	{
		IMkShaderPtr program= m_impl->parentMaterial->getProgram();

		// Auto-apply callback specific uniform bindings first
		if (callback)
		{
			for (auto it = program->getUniformBegin(); it != program->getUniformEnd(); it++)
			{
				const std::string& uniformName = it->first;
				eUniformSemantic uniformSemantic = it->second.semantic;
				eUniformDataType uniformDataType = getUniformSemanticDataType(uniformSemantic);

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
		for (auto it = m_impl->floatSources.getMap().begin(); it != m_impl->floatSources.getMap().end(); ++it)
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
		for (auto it = m_impl->float2Sources.getMap().begin(); it != m_impl->float2Sources.getMap().end(); ++it)
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
		for (auto it = m_impl->float3Sources.getMap().begin(); it != m_impl->float3Sources.getMap().end(); ++it)
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
		for (auto it = m_impl->float4Sources.getMap().begin(); it != m_impl->float4Sources.getMap().end(); ++it)
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
		for (auto it = m_impl->mat4Sources.getMap().begin(); it != m_impl->mat4Sources.getMap().end(); ++it)
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
		for (auto it = m_impl->textureSources.getMap().begin(); it != m_impl->textureSources.getMap().end(); ++it)
		{
			const std::string& uniformName= it->first;
			IMkTexturePtr texture= it->second;
			int textureUnit= 0;

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

	return MkScopedMaterialInstanceBinding(shared_from_this(), unboundUniforms, bMaterialInstanceFailure);
}

void MkMaterialInstance::unbindMaterialInstance() const
{
	if (m_impl->parentMaterial == nullptr)
		return;

	// Unbind all textures
	for (auto it = m_impl->textureSources.getMap().begin(); it != m_impl->textureSources.getMap().end(); ++it)
	{
		const std::string& uniformName = it->first;
		IMkTexturePtr texture = it->second;

		int textureUnit= 0;
		if (m_impl->parentMaterial->getProgram()->getUniformTextureUnit(uniformName, textureUnit))
		{
			it->second->clearTexture(textureUnit);
		}
	}
}

// -- GlScopedMaterialInstanceBinding ------
MkScopedMaterialInstanceBinding::~MkScopedMaterialInstanceBinding()
{
	if (m_boundMaterialInstance != nullptr)
	{
		m_boundMaterialInstance->unbindMaterialInstance();
	}
}