#pragma once

#include "NamedValueTable.h"
#include "GlProgramConstants.h"
#include "GlScopedMaterialBinding.h"

#include <string>
#include <memory>
#include <map>

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class GlTexture;
typedef std::shared_ptr<GlTexture> GlTexturePtr;

class GlProgram;
typedef std::shared_ptr<GlProgram> GlProgramPtr;

class GlMaterial
{
public:
	GlMaterial() = default;
	GlMaterial(const std::string& name, GlProgramPtr program);

	const std::string& getName() const { return m_name; }

	void setProgram(GlProgramPtr program);
	GlProgramPtr getProgram() const;

	bool setFloatBySemantic(eUniformSemantic semantic, float value);
	bool getFloatBySemantic(eUniformSemantic semantic, float& outValue) const;
	bool setFloatByUniformName(const std::string uniformName, float value);
	bool getFloatByUniformName(const std::string uniformName, float &outValue) const;

	bool setVec2BySemantic(eUniformSemantic semantic, const glm::vec2& value);
	bool getVec2BySemantic(eUniformSemantic semantic, glm::vec2& outValue) const;
	bool setVec2ByUniformName(const std::string uniformName, const glm::vec2& value);
	bool getVec2ByUniformName(const std::string uniformName, glm::vec2& outValue) const;

	bool setVec3BySemantic(eUniformSemantic semantic, const glm::vec3& value);
	bool getVec3BySemantic(eUniformSemantic semantic, glm::vec3& outValue) const;
	bool setVec3ByUniformName(const std::string uniformName, const glm::vec3& value);
	bool getVec3ByUniformName(const std::string uniformName, glm::vec3& outValue) const;

	bool setVec4BySemantic(eUniformSemantic semantic, const glm::vec4& value);
	bool getVec4BySemantic(eUniformSemantic semantic, glm::vec4& outValue) const;
	bool setVec4ByUniformName(const std::string uniformName, const glm::vec4& value);
	bool getVec4ByUniformName(const std::string uniformName, glm::vec4& outValue) const;

	bool setMat4BySemantic(eUniformSemantic semantic, const glm::mat4& value);
	bool getMat4BySemantic(eUniformSemantic semantic, glm::mat4& outValue) const;
	bool setMat4ByUniformName(const std::string uniformName, const glm::mat4& value);
	bool getMat4ByUniformName(const std::string uniformName, glm::mat4& outValue) const;

	bool setTextureBySemantic(eUniformSemantic semantic, GlTexturePtr texture);
	bool getTextureBySemantic(eUniformSemantic semantic, GlTexturePtr& outTexture) const;
	bool setTextureByUniformName(const std::string uniformName, GlTexturePtr texture);
	bool getTextureByUniformName(const std::string uniformName, GlTexturePtr& outTexture) const;

	GlScopedMaterialBinding bindMaterial() const;

protected:
	friend class GlScopedMaterialBinding;
	void unbindMaterial() const;

private:
	std::string m_name;
	GlProgramPtr m_program = nullptr;

	// Program Parameters
	NamedValueTable<float> m_floatSources;
	NamedValueTable<glm::vec2> m_float2Sources;
	NamedValueTable<glm::vec3> m_float3Sources;
	NamedValueTable<glm::vec4> m_float4Sources;
	NamedValueTable<glm::mat4> m_mat4Sources;
	NamedValueTable<GlTexturePtr> m_textureSources;
};
