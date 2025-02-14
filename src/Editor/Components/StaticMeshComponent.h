#pragma once

#include "ComponentFwd.h"
#include "MulticastDelegate.h"
#include "IMkMesh.h"
#include "ObjectSystemFwd.h"
#include "MikanRendererFwd.h"
#include "SceneComponent.h"

class StaticMeshComponent : public SceneComponent
{
public:
	StaticMeshComponent(MikanObjectWeakPtr owner);

	IMkStaticMeshInstancePtr getStaticMesh() const;
	void setStaticMesh(IMkStaticMeshInstancePtr meshInstance);

	void extractRenderGeometry(struct MikanTriagulatedMesh& outRenderGeometry);

	MulticastDelegate<void(StaticMeshComponentWeakPtr meshComponent)> OnMeshChanged;
};
