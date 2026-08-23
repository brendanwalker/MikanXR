#pragma once

#include "MikanRendererFwd.h"

#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>

struct KdTreeRaycastRequest
{
	glm::mat4 worldMatrix= glm::mat4(1.f);
	glm::vec3 origin= glm::vec3(0.f);
	glm::vec3 direction= glm::vec3(0.f);
	bool debugDraw= false;
};

struct KdTreeRaycastResult
{
	bool hit= false;
	float distance= -1.f;
	glm::vec3 position= glm::vec3(0.f);
	glm::vec3 normal= glm::vec3(0.f);
	int triangleIndex= -1;
};

struct KdTreeClosestPointResult
{
	bool valid= false;
	float distance= -1.f;               // local-space distance from the query point to the surface
	glm::vec3 position= glm::vec3(0.f); // local-space closest point on the mesh surface
	glm::vec3 normal= glm::vec3(0.f);   // local-space normal of the winning triangle
	int triangleIndex= -1;
};

class StaticMeshKdTree
{
public:
	StaticMeshKdTree();
	virtual ~StaticMeshKdTree();

	bool init();
	void dispose();

	bool isInitialized() const;

	bool setMesh(IMkMeshConstPtr mesh);
	inline IMkMeshConstPtr getMesh() const;

	bool getLocalAABB(glm::vec3& outMin, glm::vec3& outMax) const;
	bool computeRayIntersection(const KdTreeRaycastRequest& request, KdTreeRaycastResult& result) const;
	bool computeClosestVertex(const glm::vec3& localPoint, const int triangleIndex, glm::vec3& closestVertex) const;

	// Find the closest point on the mesh surface to a query point, both expressed in mesh-local space.
	bool computeClosestPoint(const glm::vec3& localPoint, KdTreeClosestPointResult& result) const;

private:
	class KdTreeMeshAccessor* m_meshAccessor= nullptr;
	class KdTreeData* m_treeData= nullptr;
};
