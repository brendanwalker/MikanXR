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

bool MeshColliderComponent::getBoundingSphere(glm::vec3& outCenter, float& outRadius) const
{
	if (!m_kdTree)
		return false;

	glm::vec3 localMin, localMax;
	if (!m_kdTree->getLocalAABB(localMin, localMax))
		return false;

	const glm::mat4& worldXform= getWorldTransform();
	const glm::vec3 minBounds= worldXform * glm::vec4(localMin, 1.f);
	const glm::vec3 maxBounds= worldXform * glm::vec4(localMax, 1.f);

	outCenter = (minBounds + maxBounds) * 0.5f;
	outRadius = glm::distance(minBounds, outCenter);

	return true;
}

bool MeshColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	if (!m_bEnabled || !m_kdTree)
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

		// Compute closest vertex to the collision point in the local space of the mesh
		glm::vec3 localClosestVertex;
		if (m_kdTree->computeClosestVertex(
			kdTreeResult.position,
			kdTreeResult.triangleIndex,
			localClosestVertex))
		{
			outResult.closestVertexLocal = localClosestVertex;
			outResult.closestVertexWorld = worldXform * glm::vec4(localClosestVertex, 1.f);
			outResult.closestVertexValid = true;
		}
		else
		{
			outResult.closestVertexLocal = glm::vec3(0.f);
			outResult.closestVertexWorld = outResult.hitLocation;
			outResult.closestVertexValid = false;
		}
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