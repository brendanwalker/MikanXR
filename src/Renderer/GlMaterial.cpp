#include "GlCamera.h"
#include "GlMaterial.h"
#include "GlProgram.h"
#include "GlScene.h"
#include "GlTexture.h"

GlMaterial::GlMaterial(
	const std::string& name, 
	GlProgramPtr program)
	: m_name(name)
	, m_program(program)
{
}

void GlMaterial::setProgram(GlProgramPtr program) 
{ 
	m_program = program; 
}

GlProgramPtr GlMaterial::getProgram() const 
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

GlScopedMaterialBinding GlMaterial::bindMaterial(
	GlSceneConstPtr scene,
	GlCameraConstPtr camera) const
{	
	bool bMaterialFailure= false;
	UniformNameSet unboundUniformNames;

	if (m_program->bindProgram())
	{
		for (auto it = m_program->getUniformBegin(); it != m_program->getUniformEnd(); ++it)
		{
			const std::string& uniformName= it->first;
			eUniformSemantic uniformSemantic= it->second.semantic;
			eUniformDataType uniformDataType= GlProgram::getUniformSemanticDataType(uniformSemantic);
			bool bIsBound= false;

			switch (uniformDataType)
			{
			case eUniformDataType::datatype_float:
				{
					float value;
					if (m_floatSources.tryGetValue(uniformName, value))
					{
						bIsBound = m_program->setFloatUniform(uniformName, value);
						bMaterialFailure= !bIsBound;
					}
				}
				break;
			case eUniformDataType::datatype_float2:
				{
					glm::vec2 value;
					if (m_float2Sources.tryGetValue(uniformName, value))
					{
						bIsBound= m_program->setVector2Uniform(uniformName, value);
						bMaterialFailure= !bIsBound;
					}
				}
				break;
			case eUniformDataType::datatype_float3:
				{
					glm::vec3 value;
					if (m_float3Sources.tryGetValue(uniformName, value))
					{
						bIsBound= m_program->setVector3Uniform(uniformName, value);
						bMaterialFailure= !bIsBound;
					}
					else
					{
						switch (uniformSemantic)
						{
							case eUniformSemantic::cameraPosition:
								if (camera != nullptr)
								{
									value= camera->getCameraPositionFromViewMatrix();
									bIsBound= m_program->setVector3Uniform(uniformName, value);
									bMaterialFailure= !bIsBound;
								}
								else
								{
									bMaterialFailure= true;
								}
								break;
							case eUniformSemantic::lightDirection:
								if (scene != nullptr)
								{
									value = scene->getLightDirection();
									bIsBound= m_program->setVector3Uniform(uniformName, value);
									bMaterialFailure= !bIsBound;
								}
								else
								{
									bMaterialFailure = true;
								}
								break;
						}
					}
				}
				break;
			case eUniformDataType::datatype_float4:
				{
					glm::vec4 value;
					if (m_float4Sources.tryGetValue(uniformName, value))
					{
						bIsBound= m_program->setVector4Uniform(uniformName, value);
						bMaterialFailure= !bIsBound;
					}
					else
					{
						switch (uniformSemantic)
						{
							case eUniformSemantic::lightColor:
								if (scene != nullptr)
								{
									value = scene->getLightColor();
									bIsBound= m_program->setVector4Uniform(uniformName, value);
									bMaterialFailure= !bIsBound;
								}
								else
								{
									bMaterialFailure = true;
								}
								break;
						}
					}
				}
				break;
			case eUniformDataType::datatype_mat4:
				{
					glm::mat4 value;
					if (m_mat4Sources.tryGetValue(uniformName, value))
					{
						bIsBound= m_program->setMatrix4x4Uniform(uniformName, value);
						bMaterialFailure= !bIsBound;
					}
					else
					{
						switch (uniformSemantic)
						{
							case eUniformSemantic::viewMatrix:
								if (camera != nullptr)
								{
									value = camera->getViewMatrix();

									bIsBound= m_program->setMatrix4x4Uniform(uniformName, value);
									bMaterialFailure= !bIsBound;
								}
								else
								{
									bMaterialFailure= true;
								}
								break;
							case eUniformSemantic::projectionMatrix:
								if (camera != nullptr)
								{
									value = camera->getProjectionMatrix();

									bIsBound= m_program->setMatrix4x4Uniform(uniformName, value);
									bMaterialFailure = !bIsBound;

								}
								else
								{
									bMaterialFailure = true;
								}
								break;
						}
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
						bIsBound = true;

						if (!m_program->setTextureUniform(uniformName) ||
							!texture->bindTexture(textureUnit))
						{
							bMaterialFailure= true;
						}
					}
				}
				break;
			default:
				assert(false);
			}

			// Track all unbound material parameters.
			// Verify in material instance that all unbound parameters are resolved.
			if (!bIsBound)
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

	return GlScopedMaterialBinding(scene, camera, shared_from_this(), unboundUniformNames, bMaterialFailure);
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