#include "GlMaterial.h"
#include "IMkShader.h"
#include "IMkTexture.h"

GlMaterial::GlMaterial(
	const std::string& name, 
	IMkShaderPtr program)
	: m_name(name)
	, m_program(program)
{
}

void GlMaterial::setProgram(IMkShaderPtr program) 
{ 
	m_program = program; 
}

IMkShaderPtr GlMaterial::getProgram() const 
{
	return m_program; 
}

bool GlMaterial::setFloatBySemantic(eUniformSemantic semantic, float value)
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setFloatByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterial::getFloatBySemantic(eUniformSemantic semantic, float& outValue) const
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getFloatByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterial::setFloatByUniformName(const std::string uniformName, float value)
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float)
	{
		m_floatSources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterial::getFloatByUniformName(const std::string uniformName, float& outValue) const
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float && 
		m_floatSources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool GlMaterial::setVec2BySemantic(eUniformSemantic semantic, const glm::vec2& value)
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec2ByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterial::getVec2BySemantic(eUniformSemantic semantic, glm::vec2& outValue) const
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec2ByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterial::setVec2ByUniformName(const std::string uniformName, const glm::vec2& value)
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float2)
	{
		m_float2Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterial::getVec2ByUniformName(const std::string uniformName, glm::vec2& outValue) const
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float2 &&
		m_float2Sources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool GlMaterial::setVec3BySemantic(eUniformSemantic semantic, const glm::vec3& value)
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec3ByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterial::getVec3BySemantic(eUniformSemantic semantic, glm::vec3& outValue) const
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec3ByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterial::setVec3ByUniformName(const std::string uniformName, const glm::vec3& value)
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float3)
	{
		m_float3Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterial::getVec3ByUniformName(const std::string uniformName, glm::vec3& outValue) const
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float3 &&
		m_float3Sources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool GlMaterial::setVec4BySemantic(eUniformSemantic semantic, const glm::vec4& value)
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec4ByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterial::getVec4BySemantic(eUniformSemantic semantic, glm::vec4& outValue) const
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec4ByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterial::setVec4ByUniformName(const std::string uniformName, const glm::vec4& value)
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float4)
	{
		m_float4Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterial::getVec4ByUniformName(const std::string uniformName, glm::vec4& outValue) const
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float4 &&
		m_float4Sources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool GlMaterial::setMat4BySemantic(eUniformSemantic semantic, const glm::mat4& value)
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setMat4ByUniformName(uniformName, value);
	}

	return false;
}

bool GlMaterial::getMat4BySemantic(eUniformSemantic semantic, glm::mat4& outValue) const
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getMat4ByUniformName(uniformName, outValue);
	}

	return false;
}

bool GlMaterial::setMat4ByUniformName(const std::string uniformName, const glm::mat4& value)
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_mat4)
	{
		m_mat4Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool GlMaterial::getMat4ByUniformName(const std::string uniformName, glm::mat4& outValue) const
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_mat4 &&
		m_mat4Sources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool GlMaterial::setTextureBySemantic(eUniformSemantic semantic, IMkTexturePtr texture)
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setTextureByUniformName(uniformName, texture);
	}

	return false;
}

bool GlMaterial::getTextureBySemantic(eUniformSemantic semantic, IMkTexturePtr& outTexture) const
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getTextureByUniformName(uniformName, outTexture);
	}

	return false;
}

bool GlMaterial::setTextureByUniformName(const std::string uniformName, IMkTexturePtr texture)
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_texture)
	{
		m_textureSources.setValue(uniformName, texture);
		return true;
	}

	return false;
}

bool GlMaterial::getTextureByUniformName(const std::string uniformName, IMkTexturePtr& outTexture) const
{
	eUniformDataType datatype;
	if (m_program != nullptr &&
		m_program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_texture &&
		m_textureSources.tryGetValue(uniformName, outTexture))
	{
		return true;
	}

	return false;
}

MkScopedMaterialBinding GlMaterial::bindMaterial(
	BindUniformCallback callback) const
{	
	bool bMaterialFailure= false;
	UniformNameSet unboundUniformNames;

	if (m_program->bindProgram())
	{
		for (auto it = m_program->getUniformBegin(); it != m_program->getUniformEnd(); ++it)
		{
			const std::string& uniformName= it->first;
			eUniformSemantic uniformSemantic= it->second.semantic;
			eUniformDataType uniformDataType= getUniformSemanticDataType(uniformSemantic);
			eUniformBindResult bindResult= eUniformBindResult::unbound;

			switch (uniformDataType)
			{
			case eUniformDataType::datatype_float:
				{
					float value;
					if (m_floatSources.tryGetValue(uniformName, value))
					{
						bindResult = 
							m_program->setFloatUniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_float2:
				{
					glm::vec2 value;
					if (m_float2Sources.tryGetValue(uniformName, value))
					{
						bindResult =
							m_program->setVector2Uniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_float3:
				{
					glm::vec3 value;
					if (m_float3Sources.tryGetValue(uniformName, value))
					{
						bindResult= 
							m_program->setVector3Uniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_float4:
				{
					glm::vec4 value;
					if (m_float4Sources.tryGetValue(uniformName, value))
					{
						bindResult= 
							m_program->setVector4Uniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_mat4:
				{
					glm::mat4 value;
					if (m_mat4Sources.tryGetValue(uniformName, value))
					{
						bindResult= 
							m_program->setMatrix4x4Uniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_texture:
				{
					IMkTexturePtr texture;
					int textureUnit= 0;
					if (m_textureSources.tryGetValue(uniformName, texture) && 
						m_program->getUniformTextureUnit(uniformName, textureUnit))
					{
						bindResult =
							m_program->setTextureUniform(uniformName) && texture->bindTexture(textureUnit)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			default:
				assert(false);
			}

			// Try using the binding callback if the uniform was unbound
			if (bindResult == eUniformBindResult::unbound && callback)
			{
				bindResult= callback(m_program, uniformDataType, uniformSemantic, uniformName);
			}

			// Flag if there was an error binding a material parameter of any kind
			if (bindResult == eUniformBindResult::error)
			{
				bMaterialFailure = true;
			}
			// Track all unbound material parameters.
			// Verify in material instance that all unbound parameters are resolved.
			if (bindResult == eUniformBindResult::unbound)
			{
				unboundUniformNames.insert(uniformName);
			}
		}

		// Unbind the program if we failed to bind any material parameters that should have worked
		if (bMaterialFailure)
		{
			unbindMaterial();
		}
	}
	else
	{
		bMaterialFailure= true;
	}

	return MkScopedMaterialBinding(shared_from_this(), unboundUniformNames, bMaterialFailure);
}

void GlMaterial::unbindMaterial() const
{
	// Unbind all textures
	for (auto it = m_textureSources.getMap().begin(); it != m_textureSources.getMap().end(); ++it)
	{
		const std::string& uniformName= it->first;
		IMkTexturePtr texture= it->second;

		int textureUnit= -1;
		if (m_program->getUniformTextureUnit(uniformName, textureUnit))
		{
			it->second->clearTexture(textureUnit);
		}
	}

	// Unbind the shader program
	m_program->unbindProgram();
}