#pragma once

#include "ColliderComponent.h"
#include "IMkMesh.h"
#include "MathGLM.h"
#include "ObjectFwd.h"
#include "MikanRendererFwd.h"

class StaticMeshKdTree;
using StaticMeshKdTreePtr= std::shared_ptr<StaticMeshKdTree>;

class MeshColliderComponent : public ColliderComponent
{
public:
	MeshColliderComponent(MikanObjectWeakPtr owner);

	virtual void dispose() override;

	inline static const std::string k_componentClassName= "MeshColliderComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	void setStaticMeshComponent(StaticMeshComponentWeakPtr meshComponent);
	inline StaticMeshComponentWeakPtr getStaticMeshComponent() const { return m_staticMeshWeakPtr; }

	virtual bool getBoundingSphere(glm::vec3& outCenter, float& outRadius) const override;
	virtual bool computeRayIntersection(const ColliderRaycastHitRequest& request,
										ColliderRaycastHitResult& outResult) const override;

	// Closest point on the collision mesh to a query point expressed in this component's mesh-local space.
	// The result (position/normal/distance) is also in mesh-local space. Independent of the component's
	// world transform, so callers solving for a transform (e.g. ICP) can query against their own estimate.
	bool computeClosestPointLocal(const glm::vec3& localPoint, struct KdTreeClosestPointResult& outResult) const;

	// Convenience world-space query using the component's current world transform.
	bool computeClosestPointWorld(const glm::vec3& worldPoint, glm::vec3& outWorldPoint, glm::vec3& outWorldNormal,
								  float& outWorldDistance) const;

	// Local-space AABB of the collision mesh (mesh-local coordinates), for bounds/OBB seeding.
	bool getLocalAABB(glm::vec3& outMin, glm::vec3& outMax) const;

private:
	void onStaticMeshChanged(StaticMeshComponentWeakPtr meshComponent);
	void rebuildCollisionGeometry();

	StaticMeshComponentWeakPtr m_staticMeshWeakPtr;
	StaticMeshKdTreePtr m_kdTree;
};
