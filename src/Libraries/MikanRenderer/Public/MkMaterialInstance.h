#pragma once

#include "MkShaderConstants.h"
#include "MkMaterial.h"
#include "MkScopedMaterialBinding.h"
#include "NamedValueTable.h"
#include "MkRendererFwd.h"

#include <string>
#include <map>
#include <memory>

#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/matrix_float4x4.hpp"


class MkScopedMaterialInstanceBinding
{
public:
	MkScopedMaterialInstanceBinding() : m_boundMaterialInstance(nullptr) {}
	MkScopedMaterialInstanceBinding(
		MkMaterialInstanceConstPtr materialInstance,
		UniformNameSet unboundUniformNames,
		bool bMaterialInstanceFailure) 
		: m_boundMaterialInstance(materialInstance) 
		, m_unboundUniformNames(unboundUniformNames)
		, m_bMaterialInstanceFailure(bMaterialInstanceFailure)
	{}
	virtual ~MkScopedMaterialInstanceBinding();

	inline MkMaterialInstanceConstPtr getBoundMaterialInstance() const { return m_boundMaterialInstance; }
	inline const UniformNameSet& getUnboundUniforms() const { return m_unboundUniformNames; }
	inline operator bool() const { return !m_bMaterialInstanceFailure; }

private:
	MkMaterialInstanceConstPtr m_boundMaterialInstance = nullptr;
	UniformNameSet m_unboundUniformNames;
	bool m_bMaterialInstanceFailure= false;
};

class MkMaterialInstance : public std::enable_shared_from_this<MkMaterialInstance>
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
