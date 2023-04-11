#pragma once

#include "MikanColliderComponent.h"
#include "IGlMesh.h"
#include "MathGLM.h"

#include <memory>

class MikanObject;
typedef std::weak_ptr<MikanObject> MikanObjectWeakPtr;

class GlStaticMeshInstance;
typedef std::shared_ptr<GlStaticMeshInstance> GlStaticMeshInstancePtr;

class MikanStaticMeshComponent;
typedef std::shared_ptr<MikanStaticMeshComponent> MikanStaticMeshComponentPtr;
typedef std::weak_ptr<MikanStaticMeshComponent> MikanStaticMeshComponentWeakPtr;

class MikanBoxColliderComponent : public MikanColliderComponent
{
public:
	MikanBoxColliderComponent(MikanObjectWeakPtr owner);

	void setHalfExtents(const glm::vec3 halfExtents) { m_halfExtents= halfExtents; }

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	glm::vec3 m_halfExtents;
};
typedef std::shared_ptr<MikanBoxColliderComponent> MikanBoxColliderComponentPtr;
typedef std::weak_ptr<MikanBoxColliderComponent> MikanBoxColliderComponentWeakPtr;