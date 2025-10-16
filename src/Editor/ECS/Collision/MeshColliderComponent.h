#pragma once

#include "ColliderComponent.h"
#include "IMkMesh.h"
#include "MathGLM.h"
#include "ObjectFwd.h"
#include "MikanRendererFwd.h"

class StaticMeshKdTree;
using StaticMeshKdTreePtr = std::shared_ptr<StaticMeshKdTree>;

class MeshColliderComponent : public ColliderComponent
{
public:
	MeshColliderComponent(MikanObjectWeakPtr owner);

	virtual void dispose() override;

	inline static const std::string k_componentClassName = "MeshColliderComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	void setStaticMeshComponent(StaticMeshComponentWeakPtr meshComponent);
	inline StaticMeshComponentWeakPtr getStaticMeshComponent() const { return m_staticMeshWeakPtr; }

	virtual bool getBoundingSphere(glm::vec3& outCenter, float& outRadius) const override;
	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	void onStaticMeshChanged(StaticMeshComponentWeakPtr meshComponent);
	void rebuildCollisionGeometry();

	StaticMeshComponentWeakPtr m_staticMeshWeakPtr;
	StaticMeshKdTreePtr m_kdTree;
};
