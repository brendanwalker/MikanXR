#include "MeshColliderComponent.h"
#include "MikanObject.h"
#include "StaticMeshComponent.h"
#include "MulticastDelegate.h"
#include "GlStaticMeshInstance.h"
#include "GlMaterialInstance.h"
#include "GlMaterial.h"
#include "GlProgram.h"
#include "Logger.h"
#include "StaticMeshKdTree.h"

#include <glm/gtx/intersect.hpp>

MeshColliderComponent::MeshColliderComponent(MikanObjectWeakPtr owner)
	: ColliderComponent(owner)
	, m_kdTree(std::make_shared<StaticMeshKdTree>())
{
}

void MeshColliderComponent::dispose()
{
	m_kdTree= nullptr;

	auto staticMeshPtr= m_staticMeshWeakPtr.lock();
	if (staticMeshPtr)
	{
		staticMeshPtr->OnMeshChanged-= MakeDelegate(this, &MeshColliderComponent::onStaticMeshChanged);
	}

	m_staticMeshWeakPtr.reset();

	ColliderComponent::dispose();
}

bool MeshColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	if (!m_bEnabled)
		return false;

	// Get the world transform of this component
	const glm::mat4& worldXform= getWorldTransform();
	const glm::mat4& invWorldXform= glm::inverse(worldXform);

	KdTreeRaycastRequest kdTreeRequest;
	kdTreeRequest.worldMatrix= worldXform;
	kdTreeRequest.origin= invWorldXform * glm::vec4(request.rayOrigin, 1.f);
	kdTreeRequest.direction= invWorldXform * glm::vec4(request.rayDirection, 0.f);
	kdTreeRequest.debugDraw= false;
	KdTreeRaycastResult kdTreeResult;

	if (m_kdTree->computeRayIntersection(kdTreeRequest, kdTreeResult))
	{
		outResult.hitValid = true;
		outResult.hitLocation = worldXform * glm::vec4(kdTreeResult.position, 1.f);
		outResult.hitNormal = worldXform * glm::vec4(kdTreeResult.normal, 0.f);
		outResult.hitDistance = glm::distance(request.rayOrigin, outResult.hitLocation);
		outResult.hitPriority = m_priority;
		outResult.hitComponent =
			std::const_pointer_cast<ColliderComponent>(
				getSelfPtr<const ColliderComponent>());
	}

	return outResult.hitValid;
}

void MeshColliderComponent::setStaticMeshComponent(StaticMeshComponentWeakPtr staticMeshWeakPtr)
{
	if (m_staticMeshWeakPtr.lock() != staticMeshWeakPtr.lock())
	{
		auto oldStaticMeshPtr = m_staticMeshWeakPtr.lock();
		if (oldStaticMeshPtr)
		{
			oldStaticMeshPtr->OnMeshChanged -= MakeDelegate(this, &MeshColliderComponent::onStaticMeshChanged);
		}

		m_staticMeshWeakPtr = staticMeshWeakPtr;

		auto staticMeshPtr= staticMeshWeakPtr.lock();
		if (staticMeshPtr)
		{
			staticMeshPtr->OnMeshChanged += MakeDelegate(this, &MeshColliderComponent::onStaticMeshChanged);
		}

		rebuildCollisionGeometry();
	}
}

void MeshColliderComponent::onStaticMeshChanged(StaticMeshComponentWeakPtr meshComponentWeakPtr)
{
	assert(m_staticMeshWeakPtr.lock() != meshComponentWeakPtr.lock());
	rebuildCollisionGeometry();
}

void MeshColliderComponent::rebuildCollisionGeometry()
{
	m_kdTree->dispose();

	auto staticMeshPtr = m_staticMeshWeakPtr.lock();
	if (!staticMeshPtr)
		return;

	auto glMeshInstancePtr = staticMeshPtr->getStaticMesh();
	if (!glMeshInstancePtr)
		return;

	auto glMeshDataPtr = glMeshInstancePtr->getMesh();
	if (!glMeshDataPtr)
		return;

	m_kdTree->setMesh(glMeshDataPtr);
	if (!m_kdTree->init())
	{
		MIKAN_LOG_ERROR("MeshColliderComponent::rebuildCollisionGeometry") 
			<< "Failed to build kd-tree for mesh collider " << getName();
	}
}