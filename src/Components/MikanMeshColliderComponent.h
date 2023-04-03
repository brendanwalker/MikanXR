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

	virtual void init() override;
	virtual void dispose() override;

	virtual bool computeRayIntersection(
		const ColliderRaycastHitRequest& request,
		ColliderRaycastHitResult& outResult) const override;

private:
	void onStaticMeshChanged(MikanStaticMeshComponentWeakPtr meshComponent);
	void rebuildCollionGeometry();

	MikanStaticMeshComponentWeakPtr m_staticMeshPtr;
	std::vector<GlmTriangle> m_meshTriangles;
	glm::vec3 m_meshCenterPoint;
	glm::vec3 m_meshMaxCornerPoint;
};