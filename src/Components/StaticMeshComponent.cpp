#include "GlModelResourceManager.h"
#include "GlStaticMeshInstance.h"
#include "StaticMeshComponent.h"
#include "MikanObject.h"
#include "Renderer.h"

StaticMeshComponent::StaticMeshComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
}

GlStaticMeshInstancePtr StaticMeshComponent::getStaticMesh() const 
{ 
	return std::dynamic_pointer_cast<GlStaticMeshInstance>(m_renderable); 
}

void StaticMeshComponent::setStaticMesh(GlStaticMeshInstancePtr meshInstance)
{
	m_renderable= meshInstance;
	if (OnMeshChanged)
	{
		OnMeshChanged(getSelfWeakPtr<StaticMeshComponent>());
	}
}
