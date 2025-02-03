#pragma once

#include "ComponentFwd.h"
#include "MulticastDelegate.h"
#include "IGlMesh.h"
#include "ObjectSystemFwd.h"
#include "RendererFwd.h"
#include "SceneComponent.h"

class StaticMeshComponent : public SceneComponent
{
public:
	StaticMeshComponent(MikanObjectWeakPtr owner);

	GlStaticMeshInstancePtr getStaticMesh() const;
	void setStaticMesh(GlStaticMeshInstancePtr meshInstance);

	void extractRenderGeometry(struct MikanTriagulatedMesh& outRenderGeometry);

	MulticastDelegate<void(StaticMeshComponentWeakPtr meshComponent)> OnMeshChanged;
};
