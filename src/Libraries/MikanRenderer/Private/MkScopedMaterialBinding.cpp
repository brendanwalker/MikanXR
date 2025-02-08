#include "MkScopedMaterialBinding.h"
#include "GlMaterial.h"

MkScopedMaterialBinding::~MkScopedMaterialBinding()
{
	if (m_boundMaterial != nullptr)
	{
		m_boundMaterial->unbindMaterial();
	}
}