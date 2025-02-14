#pragma once

#include "MkShaderConstants.h"
#include "MkMaterial.h"
#include "MkScopedMaterialBinding.h"
#include "NamedValueTable.h"
#include "MkRendererFwd.h"
#include "MkRendererExport.h"

#include <string>
#include <map>
#include <memory>

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"


class MIKAN_RENDERER_CLASS MkScopedMaterialInstanceBinding
{
public:
	MkScopedMaterialInstanceBinding();
	MkScopedMaterialInstanceBinding(
		const MkMaterialInstance* materialInstance,
		UniformNameSet unboundUniformNames,
		bool bMaterialInstanceFailure);
	virtual ~MkScopedMaterialInstanceBinding();

	const MkMaterialInstance* getBoundMaterialInstance() const;
	const UniformNameSet& getUnboundUniforms() const;
	operator bool() const;

private:
	struct MkScopedMaterialInstanceBindingImpl* m_impl;
};

class MIKAN_RENDERER_CLASS MkMaterialInstance 
{
public:
	MkMaterialInstance();
	MkMaterialInstance(MkMaterialConstPtr material);
	MkMaterialInstance(MkMaterialInstanceConstPtr materialInstance);

	MkMaterialConstPtr getMaterial() const;

	bool setFloatBySemantic(eUniformSemantic semantic, float value);
	bool getFloatBySemantic(eUniformSemantic semantic, float& outValue) const;
	bool setFloatByUniformName(const std::string uniformName, float value);
	bool getFloatByUniformName(const std::string uniformName, float& outValue) const;

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

	MkScopedMaterialInstanceBinding bindMaterialInstance(
		const MkScopedMaterialBinding& materialBinding,
		BindUniformCallback callback= BindUniformCallback()) const;

protected: 
	friend class MkScopedMaterialInstanceBinding;
	void unbindMaterialInstance() const;

private:
	struct MkMaterialInstanceImpl* m_impl;
};
