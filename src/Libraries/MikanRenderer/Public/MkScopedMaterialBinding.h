#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"

#include <string>
#include <set>

using UniformNameSet = std::set<std::string>;

class MIKAN_RENDERER_CLASS MkScopedMaterialBinding
{
public:
	MkScopedMaterialBinding();
	MkScopedMaterialBinding(
		const class MkMaterial* material,
		UniformNameSet unboundUniformNames,
		bool bMaterialFailure);
	virtual ~MkScopedMaterialBinding();

	const MkMaterial* getBoundMaterial() const;
	const UniformNameSet& getUnboundUniforms() const;
	operator bool() const;

private:
	struct MkScopedMaterialBindingImpl* m_impl;
};
