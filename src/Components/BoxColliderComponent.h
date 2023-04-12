#pragma once

#include "ColliderComponent.h"
#include "IGlMesh.h"
#include "MathGLM.h"

#include <memory>

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class GlStaticMeshInstance;
typedef std::shared_ptr<GlStaticMeshInstance> GlStaticMeshInstancePtr;

class StaticMeshComponent;
typedef std::shared_ptr<StaticMeshComponent> StaticMeshComponentPtr;
typedef std::weak_ptr<StaticMeshComponent> StaticMeshComponentWeakPtr;

class BoxColliderComponent : public ColliderComponent
{
public:
	BoxColliderComponent(MikanObjectWeakPtr owner);

	void setHalfExtents(const glm::vec3 halfExtents) { m_halfExtents= halfExtents; }

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	glm::vec3 m_halfExtents;
};
typedef std::shared_ptr<BoxColliderComponent> BoxColliderComponentPtr;
typedef std::weak_ptr<BoxColliderComponent> BoxColliderComponentWeakPtr;