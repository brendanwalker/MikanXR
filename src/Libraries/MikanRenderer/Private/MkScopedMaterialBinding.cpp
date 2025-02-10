#include "MkScopedMaterialBinding.h"
#include "MkMaterial.h"

struct MkScopedMaterialBindingImpl
{
	const MkMaterial* boundMaterial;
	UniformNameSet unboundUniformNames;
	bool bMaterialFailure;
};

MkScopedMaterialBinding::MkScopedMaterialBinding()
	: m_impl(new MkScopedMaterialBindingImpl())
{
	m_impl->boundMaterial= nullptr;
	m_impl->unboundUniformNames= UniformNameSet();
	m_impl->bMaterialFailure= true;
}

MkScopedMaterialBinding::MkScopedMaterialBinding(
	const MkMaterial* material,
	UniformNameSet unboundUniformNames,
	bool bMaterialFailure)
	: m_impl(new MkScopedMaterialBindingImpl())
{
	m_impl->boundMaterial= material;
	m_impl->unboundUniformNames= unboundUniformNames;
	m_impl->bMaterialFailure= bMaterialFailure;
}

MkScopedMaterialBinding::~MkScopedMaterialBinding()
{
	if (m_impl->boundMaterial != nullptr)
	{
		m_impl->boundMaterial->unbindMaterial();
	}
}

const MkMaterial* MkScopedMaterialBinding::getBoundMaterial() const
{ 
	return m_impl->boundMaterial; 
}

const UniformNameSet& MkScopedMaterialBinding::getUnboundUniforms() const 
{ 
	return m_impl->unboundUniformNames; 
}

MkScopedMaterialBinding::operator bool() const 
{ 
	return !m_impl->bMaterialFailure; 
}