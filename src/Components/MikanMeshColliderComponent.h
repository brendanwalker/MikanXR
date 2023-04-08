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

class MikanMeshColliderComponent : public MikanColliderComponent
{
public:
	MikanMeshColliderComponent(MikanObjectWeakPtr owner);

	virtual void dispose() override;

	void setStaticMeshComponent(MikanStaticMeshComponentWeakPtr meshComponent);
	inline MikanStaticMeshComponentWeakPtr getStaticMeshComponent() const { return m_staticMeshWeakPtr; }

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	void onStaticMeshChanged(MikanStaticMeshComponentWeakPtr meshComponent);
	void rebuildCollionGeometry();

	MikanStaticMeshComponentWeakPtr m_staticMeshWeakPtr;
	std::vector<GlmTriangle> m_meshTriangles;
	glm::vec3 m_meshCenterPoint;
	glm::vec3 m_meshMaxCornerPoint;
};
typedef std::shared_ptr<MikanMeshColliderComponent> MikanMeshColliderComponentPtr;
typedef std::weak_ptr<MikanMeshColliderComponent> MikanMeshColliderComponentWeakPtr;