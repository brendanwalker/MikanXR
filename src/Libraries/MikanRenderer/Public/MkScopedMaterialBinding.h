#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"

#include <string>
#include <set>

using UniformNameSet = std::set<std::string>;

class MIKAN_RENDERER_CLASS MkScopedMaterialBinding
{
public:
	MkScopedMaterialBinding() = default;
	MkScopedMaterialBinding(
		GlMaterialConstPtr material,
		UniformNameSet unboundUniformNames,
		bool bMaterialFailure)
		: m_boundMaterial(material) 
		, m_unboundUniformNames(unboundUniformNames)
		, m_bMaterialFailure(bMaterialFailure)
	{}
	virtual ~MkScopedMaterialBinding();

	inline GlMaterialConstPtr getBoundMaterial() const { return m_boundMaterial; }
	inline const UniformNameSet& getUnboundUniforms() const { return m_unboundUniformNames; }
	inline operator bool() const { return !m_bMaterialFailure; }

private:
	GlMaterialConstPtr m_boundMaterial;
	UniformNameSet m_unboundUniformNames;
	bool m_bMaterialFailure;
};
