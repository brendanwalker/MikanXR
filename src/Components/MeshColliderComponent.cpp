#include "MeshColliderComponent.h"
#include "MikanObject.h"
#include "StaticMeshComponent.h"
#include "MulticastDelegate.h"
#include "GlStaticMeshInstance.h"

#include <glm/gtx/intersect.hpp>

MeshColliderComponent::MeshColliderComponent(MikanObjectWeakPtr owner)
	: ColliderComponent(owner)
{
}

void MeshColliderComponent::dispose()
{
	auto staticMeshPtr= m_staticMeshWeakPtr.lock();
	if (staticMeshPtr)
	{
		staticMeshPtr->OnMeshChanged-= MakeDelegate(this, &MeshColliderComponent::onStaticMeshChanged);
	}

	m_staticMeshWeakPtr.reset();
}

bool MeshColliderComponent::computeRayIntersection(
	const ColliderRaycastHitRequest& request,
	ColliderRaycastHitResult& outResult) const
{
	outResult.hitValid = false;
	outResult.hitLocation = glm::vec3(0.f);
	outResult.hitNormal = glm::vec3(0.f);
	outResult.hitDistance = -1.f;

	// Get the world transform of this component
	const glm::mat4& worldXform= getWorldTransform();

	// Apply component world transform to bounds geometry
	const glm::vec3 worldBoundCenter= worldXform * glm::vec4(m_meshCenterPoint, 1.f);
	const glm::vec3 worldBoundMax= worldXform * glm::vec4(m_meshMaxCornerPoint, 1.f);
	const float worldRadius= glm::length(worldBoundMax - worldBoundCenter);

	// Early out if the ray doesn't hit the bounding sphere of the mesh
	float unusedIntDistance= 0.f;
	if (glm::intersectRaySphere(
		request.rayOrigin, request.rayDirection,
		worldBoundCenter, worldRadius * worldRadius,
		unusedIntDistance))
	{
		float closestDistance = k_real_max;
		for (const GlmTriangle& tri : m_meshTriangles)
		{
			// Apply component world transform to triangle geometry
			GlmTriangle worldTri= {
				worldXform * glm::vec4(tri.v0, 1.f),
				worldXform * glm::vec4(tri.v1, 1.f),
				worldXform * glm::vec4(tri.v2, 1.f)
			};

			// See if ray intersect with tri and is closer than any intersection found so far
			// TODO: Parallelize this intersection calculations
			float triIntDistance;
			glm::vec3 triIntPoint;
			glm::vec3 triIntNormal;
			if (glm_intersect_tri_with_ray(
				tri,
				request.rayOrigin, request.rayDirection,
				triIntDistance, triIntPoint, triIntNormal))
			{
				if (triIntDistance < closestDistance && triIntDistance >= 0.f)
				{
					outResult.hitLocation = triIntPoint;
					outResult.hitNormal = triIntNormal;
					outResult.hitDistance = triIntDistance;
					outResult.hitValid = true;
				}
			}
		}
	}

	return outResult.hitValid;
}

void MeshColliderComponent::setStaticMeshComponent(StaticMeshComponentWeakPtr staticMeshWeakPtr)
{
	if (m_staticMeshWeakPtr.lock() == staticMeshWeakPtr.lock())
	{
		dispose();

		auto staticMeshPtr= staticMeshWeakPtr.lock();
		if (staticMeshPtr)
		{
			staticMeshPtr->OnMeshChanged += MakeDelegate(this, &MeshColliderComponent::onStaticMeshChanged);
			rebuildCollionGeometry();
		}

		m_staticMeshWeakPtr = staticMeshWeakPtr;
	}
}

void MeshColliderComponent::onStaticMeshChanged(StaticMeshComponentWeakPtr meshComponentWeakPtr)
{
	assert(m_staticMeshWeakPtr.lock() == meshComponentWeakPtr.lock());
	rebuildCollionGeometry();
}

void MeshColliderComponent::rebuildCollionGeometry()
{
	m_meshTriangles.clear();
	m_meshCenterPoint= glm::vec3(0.f);
	m_meshMaxCornerPoint= glm::vec3(0.f);

	auto staticMeshPtr = m_staticMeshWeakPtr.lock();
	if (!staticMeshPtr)
		return;

	auto glMeshInstancePtr = staticMeshPtr->getStaticMesh();
	if (!glMeshInstancePtr)
		return;

	auto glMeshDataPtr = glMeshInstancePtr->getMesh();
	if (!glMeshDataPtr)
		return;

	// TODO: For the moment, we are only supporting triangulated meshes with 16-bit indices
	assert(glMeshDataPtr->getIndexPerElementCount() == 3);
	assert(glMeshDataPtr->getIndexSize() == sizeof(sizeof(uint16_t)));

	auto vertexDefinition= glMeshDataPtr->getVertexDefinition();
	auto vertexAttrib= vertexDefinition->getFirstAttributeBySemantic(eVertexSemantic::position3f);
	if (!vertexAttrib)
		return;

	const size_t triangleCount = glMeshDataPtr->getElementCount();
	if (triangleCount <= 0)
		return;

	const uint8_t* vertexData= glMeshDataPtr->getVertexData();
	const uint8_t* indexData= glMeshDataPtr->getIndexData();
	const size_t triangleStride= glMeshDataPtr->getIndexSize() * 3;
	const size_t vertexStride= vertexAttrib->stride;
	const size_t positionOffset= vertexAttrib->offset;

	// Extract the triangle and bounding points from the mesh data
	auto extractPosition3f = [vertexData, vertexStride, positionOffset](uint16_t vertexIndex) -> glm::vec3 {
		const size_t positionDataOffset= vertexStride*vertexIndex + positionOffset;
		const glm::vec3* positionAccess= (const glm::vec3*)(vertexData + positionDataOffset);

		return *positionAccess;
	};

	glm::vec3 minPoint= glm::vec3(k_real_max, k_real_max, k_real_max);
	glm::vec3 maxPoint= glm::vec3(k_real_min, k_real_min, k_real_min);
	const uint8_t* indexPtr= indexData;
	for (size_t triIndex= 0; triIndex < triangleCount; ++triIndex)
	{
		const uint16_t* indexPtrUint16= (const uint16_t*)indexPtr;
		const uint16_t i0= indexPtrUint16[0];
		const uint16_t i1= indexPtrUint16[1];
		const uint16_t i2= indexPtrUint16[2];

		GlmTriangle tri;
		tri.v0= extractPosition3f(i0);
		tri.v1= extractPosition3f(i1);
		tri.v2= extractPosition3f(i2);
		m_meshTriangles.push_back(tri);

		minPoint= glm::min(minPoint, tri.v0);
		minPoint= glm::min(minPoint, tri.v1);
		minPoint= glm::min(minPoint, tri.v2);

		maxPoint = glm::max(maxPoint, tri.v0);
		maxPoint = glm::max(maxPoint, tri.v1);
		maxPoint = glm::max(maxPoint, tri.v2);

		indexPtr+= triangleStride;
	}

	// Save off the mid and max point so that we can compute a bounding sphere later
	m_meshCenterPoint= (maxPoint + minPoint) * 0.5f;
	m_meshMaxCornerPoint= maxPoint;
}