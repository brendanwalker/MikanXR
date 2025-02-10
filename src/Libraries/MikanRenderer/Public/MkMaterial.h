#pragma once

#include "NamedValueTable.h"
#include "MkShaderConstants.h"
#include "MkScopedMaterialBinding.h"
#include "MkRendererFwd.h"
#include "MkRendererExport.h"

#include <functional>
#include <string>
#include <memory>
#include <map>

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"

// Uniform binding callback
enum class eUniformBindResult : int
{
	bound,
	unbound,
	error
};
using BindUniformCallback =
std::function<eUniformBindResult(
	std::shared_ptr<class IMkShader>, // Source program to bind the uniform for
	eUniformDataType, // Data type of the uniform
	eUniformSemantic, // Semantic of the uniform
	const std::string&)>; // Name of the uniform

class MIKAN_RENDERER_CLASS MkMaterial 
{
public:
	MkMaterial();
	MkMaterial(const std::string& name, IMkShaderPtr program);

	const std::string& getName() const;

	void setProgram(IMkShaderPtr program);
	IMkShaderPtr getProgram() const;

	const NamedValueTable<float>& getFloatSources() const;
	const NamedValueTable<glm::vec2>& getFloat2Sources() const;
	const NamedValueTable<glm::vec3>& getFloat3Sources() const;
	const NamedValueTable<glm::vec4>& getFloat4Sources() const;
	const NamedValueTable<glm::mat4>& getMat4Sources() const;
	const NamedValueTable<IMkTexturePtr>& getTextureSources() const;

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

	bool setTextureBySemantic(eUniformSemantic semantic, IMkTexturePtr texture);
	bool getTextureBySemantic(eUniformSemantic semantic, IMkTexturePtr& outTexture) const;
	bool setTextureByUniformName(const std::string uniformName, IMkTexturePtr texture);
	bool getTextureByUniformName(const std::string uniformName, IMkTexturePtr& outTexture) const;

	MkScopedMaterialBinding bindMaterial(BindUniformCallback callback= BindUniformCallback()) const;

protected:
	friend class MkScopedMaterialBinding;
	void unbindMaterial() const;

private:
	struct MkMaterialImpl* m_impl;
};
