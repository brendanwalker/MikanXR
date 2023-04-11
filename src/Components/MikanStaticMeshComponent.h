#pragma once

#include "MulticastDelegate.h"
#include "MikanSceneComponent.h"
#include "IGlMesh.h"

#include <memory>

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class GlStaticMeshInstance;
typedef std::shared_ptr<GlStaticMeshInstance> GlStaticMeshInstancePtr;

class MikanStaticMeshComponent;
typedef std::shared_ptr<MikanStaticMeshComponent> MikanStaticMeshComponentPtr;
typedef std::weak_ptr<MikanStaticMeshComponent> MikanStaticMeshComponentWeakPtr;

class MikanStaticMeshComponent : public MikanSceneComponent
{
public:
	MikanStaticMeshComponent(MikanObjectWeakPtr owner);

	GlStaticMeshInstancePtr getStaticMesh() const;
	void setStaticMesh(GlStaticMeshInstancePtr meshInstance);

	MulticastDelegate<void(MikanStaticMeshComponentWeakPtr meshComponent)> OnMeshChanged;
};
