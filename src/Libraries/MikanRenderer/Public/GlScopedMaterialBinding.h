#pragma once

#include <string>
#include <set>

#include "MkRendererFwd.h"

using UniformNameSet = std::set<std::string>;

class GlScopedMaterialBinding
{
public:
	GlScopedMaterialBinding() = default;
	GlScopedMaterialBinding(
		GlMaterialConstPtr material,
		UniformNameSet unboundUniformNames,
		bool bMaterialFailure)
		: m_boundMaterial(material) 
		, m_unboundUniformNames(unboundUniformNames)
		, m_bMaterialFailure(bMaterialFailure)
	{}
	virtual ~GlScopedMaterialBinding();

	inline GlMaterialConstPtr getBoundMaterial() const { return m_boundMaterial; }
	inline const UniformNameSet& getUnboundUniforms() const { return m_unboundUniformNames; }
	inline operator bool() const { return !m_bMaterialFailure; }

private:
	GlMaterialConstPtr m_boundMaterial;
	UniformNameSet m_unboundUniformNames;
	bool m_bMaterialFailure;
};
