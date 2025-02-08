#include "MkScopedMaterialBinding.h"
#include "MkMaterial.h"

MkScopedMaterialBinding::~MkScopedMaterialBinding()
{
	if (m_boundMaterial != nullptr)
	{
		m_boundMaterial->unbindMaterial();
	}
}