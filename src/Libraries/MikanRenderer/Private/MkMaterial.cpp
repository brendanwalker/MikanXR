#include "MkMaterial.h"
#include "IMkShader.h"
#include "IMkTexture.h"

struct MkMaterialImpl
{
	std::string name;
	IMkShaderWeakPtr program;

	// Program Parameters
	NamedValueTable<float> floatSources;
	NamedValueTable<glm::vec2> float2Sources;
	NamedValueTable<glm::vec3> float3Sources;
	NamedValueTable<glm::vec4> float4Sources;
	NamedValueTable<glm::mat4> mat4Sources;
	NamedValueTable<IMkTexturePtr> textureSources;
};

MkMaterial::MkMaterial()
	: m_impl(new MkMaterialImpl())
{
}

MkMaterial::MkMaterial(
	const std::string& name, 
	IMkShaderPtr program)
	: m_impl(new MkMaterialImpl())
{
	m_impl->name= name;
	m_impl->program= program;
}

MkMaterial::~MkMaterial()
{
	m_impl->program.reset();
	m_impl->textureSources.clear();
	delete m_impl;
}

const std::string& MkMaterial::getName() const 
{ 
	return m_impl->name; 
}

const NamedValueTable<float>& MkMaterial::getFloatSources() const 
{
	return m_impl->floatSources; 
}

const NamedValueTable<glm::vec2>& MkMaterial::getFloat2Sources() const
{
	return m_impl->float2Sources; 
}

const NamedValueTable<glm::vec3>& MkMaterial::getFloat3Sources() const 
{
	return m_impl->float3Sources; 
}

const NamedValueTable<glm::vec4>& MkMaterial::getFloat4Sources() const
{
	return m_impl->float4Sources; 
}

const NamedValueTable<glm::mat4>& MkMaterial::getMat4Sources() const
{
	return m_impl->mat4Sources; 
}

const NamedValueTable<IMkTexturePtr>& MkMaterial::getTextureSources() const
{ 
	return m_impl->textureSources; 
}

void MkMaterial::setProgram(IMkShaderPtr program) 
{ 
	m_impl->program = program; 
}

IMkShaderPtr MkMaterial::getProgram() const 
{
	return m_impl->program.lock(); 
}

bool MkMaterial::setFloatBySemantic(eUniformSemantic semantic, float value)
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setFloatByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterial::getFloatBySemantic(eUniformSemantic semantic, float& outValue) const
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getFloatByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterial::setFloatByUniformName(const std::string uniformName, float value)
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float)
	{
		m_impl->floatSources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterial::getFloatByUniformName(const std::string uniformName, float& outValue) const
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float && 
		m_impl->floatSources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool MkMaterial::setVec2BySemantic(eUniformSemantic semantic, const glm::vec2& value)
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec2ByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterial::getVec2BySemantic(eUniformSemantic semantic, glm::vec2& outValue) const
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec2ByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterial::setVec2ByUniformName(const std::string uniformName, const glm::vec2& value)
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float2)
	{
		m_impl->float2Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterial::getVec2ByUniformName(const std::string uniformName, glm::vec2& outValue) const
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float2 &&
		m_impl->float2Sources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool MkMaterial::setVec3BySemantic(eUniformSemantic semantic, const glm::vec3& value)
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec3ByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterial::getVec3BySemantic(eUniformSemantic semantic, glm::vec3& outValue) const
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec3ByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterial::setVec3ByUniformName(const std::string uniformName, const glm::vec3& value)
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float3)
	{
		m_impl->float3Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterial::getVec3ByUniformName(const std::string uniformName, glm::vec3& outValue) const
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float3 &&
		m_impl->float3Sources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool MkMaterial::setVec4BySemantic(eUniformSemantic semantic, const glm::vec4& value)
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setVec4ByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterial::getVec4BySemantic(eUniformSemantic semantic, glm::vec4& outValue) const
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getVec4ByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterial::setVec4ByUniformName(const std::string uniformName, const glm::vec4& value)
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float4)
	{
		m_impl->float4Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterial::getVec4ByUniformName(const std::string uniformName, glm::vec4& outValue) const
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_float4 &&
		m_impl->float4Sources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool MkMaterial::setMat4BySemantic(eUniformSemantic semantic, const glm::mat4& value)
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setMat4ByUniformName(uniformName, value);
	}

	return false;
}

bool MkMaterial::getMat4BySemantic(eUniformSemantic semantic, glm::mat4& outValue) const
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getMat4ByUniformName(uniformName, outValue);
	}

	return false;
}

bool MkMaterial::setMat4ByUniformName(const std::string uniformName, const glm::mat4& value)
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_mat4)
	{
		m_impl->mat4Sources.setValue(uniformName, value);
		return true;
	}

	return false;
}

bool MkMaterial::getMat4ByUniformName(const std::string uniformName, glm::mat4& outValue) const
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_mat4 &&
		m_impl->mat4Sources.tryGetValue(uniformName, outValue))
	{
		return true;
	}

	return false;
}

bool MkMaterial::setTextureBySemantic(eUniformSemantic semantic, IMkTexturePtr texture)
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return setTextureByUniformName(uniformName, texture);
	}

	return false;
}

bool MkMaterial::getTextureBySemantic(eUniformSemantic semantic, IMkTexturePtr& outTexture) const
{
	std::string uniformName;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getFirstUniformNameOfSemantic(semantic, uniformName))
	{
		return getTextureByUniformName(uniformName, outTexture);
	}

	return false;
}

bool MkMaterial::setTextureByUniformName(const std::string uniformName, IMkTexturePtr texture)
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_texture)
	{
		m_impl->textureSources.setValue(uniformName, texture);
		return true;
	}

	return false;
}

bool MkMaterial::getTextureByUniformName(const std::string uniformName, IMkTexturePtr& outTexture) const
{
	eUniformDataType datatype;
	IMkShaderPtr program= getProgram();
	if (program != nullptr &&
		program->getUniformDataType(uniformName, datatype) &&
		datatype == eUniformDataType::datatype_texture &&
		m_impl->textureSources.tryGetValue(uniformName, outTexture))
	{
		return true;
	}

	return false;
}

MkScopedMaterialBinding MkMaterial::bindMaterial(
	BindUniformCallback callback) const
{	
	bool bMaterialFailure= false;
	UniformNameSet unboundUniformNames;
	IMkShaderPtr program= getProgram();

	if (program != nullptr &&
		program->bindProgram())
	{
		for (auto it = program->getUniformBegin(); it != program->getUniformEnd(); ++it)
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
					if (m_impl->floatSources.tryGetValue(uniformName, value))
					{
						bindResult = 
							program->setFloatUniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_float2:
				{
					glm::vec2 value;
					if (m_impl->float2Sources.tryGetValue(uniformName, value))
					{
						bindResult =
							program->setVector2Uniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_float3:
				{
					glm::vec3 value;
					if (m_impl->float3Sources.tryGetValue(uniformName, value))
					{
						bindResult= 
							program->setVector3Uniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_float4:
				{
					glm::vec4 value;
					if (m_impl->float4Sources.tryGetValue(uniformName, value))
					{
						bindResult= 
							program->setVector4Uniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_mat4:
				{
					glm::mat4 value;
					if (m_impl->mat4Sources.tryGetValue(uniformName, value))
					{
						bindResult= 
							program->setMatrix4x4Uniform(uniformName, value)
							? eUniformBindResult::bound
							: eUniformBindResult::error;
					}
				}
				break;
			case eUniformDataType::datatype_texture:
				{
					IMkTexturePtr texture;
					int textureUnit= 0;
					if (m_impl->textureSources.tryGetValue(uniformName, texture) && 
						program->getUniformTextureUnit(uniformName, textureUnit))
					{
						bindResult =
							program->setTextureUniform(uniformName) && texture->bindTexture(textureUnit)
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
				bindResult= callback(program, uniformDataType, uniformSemantic, uniformName);
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

	return MkScopedMaterialBinding(this, unboundUniformNames, bMaterialFailure);
}

void MkMaterial::unbindMaterial() const
{
	IMkShaderPtr program = getProgram();
	if (program != nullptr)
	{
		// Unbind all textures
		for (auto it = m_impl->textureSources.getMap().begin(); it != m_impl->textureSources.getMap().end(); ++it)
		{
			const std::string& uniformName = it->first;
			IMkTexturePtr texture = it->second;

			int textureUnit = -1;
			if (program->getUniformTextureUnit(uniformName, textureUnit))
			{
				it->second->clearTexture(textureUnit);
			}
		}

		// Unbind the shader program
		program->unbindProgram();
	}
}