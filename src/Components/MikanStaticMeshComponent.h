#pragma once

#include "MikanSceneComponent.h"
#include "IGlMesh.h"

#include <memory>

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class GlStaticMeshInstance;
typedef std::shared_ptr<GlStaticMeshInstance> GlStaticMeshInstancePtr;

class MikanStaticMeshComponent : public MikanSceneComponent
{
public:
	MikanStaticMeshComponent(MikanObjectWeakPtr owner);

	GlStaticMeshInstancePtr getStaticMesh() const;
	void setStaticMesh(GlStaticMeshInstancePtr meshInstance);
};