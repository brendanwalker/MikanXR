#include "GlScopedMaterialBinding.h"
#include "GlMaterial.h"

GlScopedMaterialBinding::~GlScopedMaterialBinding()
{
	if (m_boundMaterial != nullptr)
	{
		m_boundMaterial->unbindMaterial();
	}
}