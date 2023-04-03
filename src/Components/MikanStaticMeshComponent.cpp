#include "MikanStaticMeshComponent.h"
#include "MikanObject.h"
#include "GlStaticMeshInstance.h"

MikanStaticMeshComponent::MikanStaticMeshComponent(MikanObjectWeakPtr owner)
	: MikanSceneComponent(owner)
{
}

GlStaticMeshInstancePtr MikanStaticMeshComponent::getStaticMesh() const 
{ 
	return std::dynamic_pointer_cast<GlStaticMeshInstance>(m_renderable); 
}

void MikanStaticMeshComponent::setStaticMesh(GlStaticMeshInstancePtr meshInstance)
{
	m_renderable= meshInstance;
	if (OnMeshChanged)
	{
		OnMeshChanged(getSelfWeakPtr<MikanStaticMeshComponent>());
	}
}
