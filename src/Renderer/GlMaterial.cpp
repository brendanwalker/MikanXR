#include "GlMaterial.h"
#include "GlProgram.h"
#include "GlTexture.h"

GlMaterial::GlMaterial(
	const std::string& name, 
	GlProgram* program)
	: m_name(name)
	, m_program(program)
{
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

bool GlMaterial::setTextureBySemantic(eUniformSemantic semantic, GlTexturePtr texture)
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setTextureByUniformName(uniformName, texture);
	}

	return false;
}

bool GlMaterial::getTextureBySemantic(eUniformSemantic semantic, GlTexturePtr& outTexture) const
{
	std::string uniformName;
	if (m_program != nullptr &&
		m_program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getTextureByUniformName(uniformName, outTexture);
	}

	return false;
}

bool GlMaterial::setTextureByUniformName(const std::string uniformName, GlTexturePtr texture)
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

bool GlMaterial::getTextureByUniformName(const std::string uniformName, GlTexturePtr& outTexture) const
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

GlScopedMaterialBinding GlMaterial::bindMaterial() const
{	
	bool bSuccess= true;

	if (m_program->bindProgram())
	{
		for (auto it = m_program->getUniformBegin(); it != m_program->getUniformEnd(); ++it)
		{
			const std::string& uniformName= it->first;
			eUniformSemantic uniformSemantic= it->second.semantic;
			eUniformDataType uniformDataType= GlProgram::getUniformSemanticDataType(uniformSemantic);

			switch (uniformDataType)
			{
			case eUniformDataType::datatype_float:
				{
					float value;
					if (m_floatSources.tryGetValue(uniformName, value))
					{
						m_program->setFloatUniform(uniformName, value);
					}
					else
					{
						bSuccess = false;
					}
				}
				break;
			case eUniformDataType::datatype_float2:
				{
					glm::vec2 value;
					if (m_float2Sources.tryGetValue(uniformName, value))
					{
						m_program->setVector2Uniform(uniformName, value);
					}
					else
					{
						bSuccess = false;
					}
				}
				break;
			case eUniformDataType::datatype_float3:
				{
					glm::vec3 value;
					if (m_float3Sources.tryGetValue(uniformName, value))
					{
						m_program->setVector3Uniform(uniformName, value);
					}
					else
					{
						bSuccess = false;
					}
				}
				break;
			case eUniformDataType::datatype_float4:
				{
					glm::vec4 value;
					if (m_float4Sources.tryGetValue(uniformName, value))
					{
						m_program->setVector4Uniform(uniformName, value);
					}
					else
					{
						bSuccess = false;
					}
				}
				break;
			case eUniformDataType::datatype_mat4:
				{
					glm::mat4 value;
					if (m_mat4Sources.tryGetValue(uniformName, value))
					{
						m_program->setMatrix4x4Uniform(uniformName, value);
					}
					else
					{
						bSuccess = false;
					}
				}
				break;
			case eUniformDataType::datatype_texture:
				{
					GlTexturePtr texture;
					int textureUnit;
					if (m_textureSources.tryGetValue(uniformName, texture) && 
						GlProgram::getTextureUniformUnit(uniformSemantic, textureUnit))
					{
						bSuccess&= 
							m_program->setTextureUniform(uniformName) &&
							texture->bindTexture(textureUnit);
					}
					else
					{
						bSuccess= false;
					}
				}
				break;
			default:
				assert(false);
			}
		}

		// Unbind the program and any bound textures if we failed to bind all parameters
		if (!bSuccess)
		{
			unbindMaterial();
		}
	}
	else
	{
		bSuccess= false;
	}

	return GlScopedMaterialBinding(bSuccess ? this : nullptr);
}

void GlMaterial::unbindMaterial() const
{
	// Unbind all textures
	for (auto it = m_textureSources.getMap().begin(); it != m_textureSources.getMap().end(); ++it)
	{
		const std::string& uniformName= it->first;
		GlTexturePtr texture= it->second;

		int textureUnit= -1;
		if (m_program->getUniformTextureUnit(uniformName, textureUnit))
		{
			it->second->clearTexture(textureUnit);
		}
	}

	// Unbind the shader program
	m_program->unbindProgram();
}

// -- GlMaterialBinding ------
GlScopedMaterialBinding::~GlScopedMaterialBinding()
{
	if (m_boundMaterial != nullptr)
	{
		m_boundMaterial->unbindMaterial();
	}
}