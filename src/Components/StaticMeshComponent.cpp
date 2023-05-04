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
	return std::dynamic_pointer_cast<GlStaticMeshInstance>(m_sceneRenderable); 
}

void StaticMeshComponent::setStaticMesh(GlStaticMeshInstancePtr meshInstance)
{
	m_sceneRenderable= meshInstance;
	if (OnMeshChanged)
	{
		StaticMeshComponentWeakPtr meshComponent= getSelfWeakPtr<StaticMeshComponent>();

		OnMeshChanged(meshComponent);
	}
}
