#pragma once

#include "RendererFwd.h"

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

class StaticMeshKdTree
{
public:
	StaticMeshKdTree();
	virtual ~StaticMeshKdTree();

	bool init();
	void dispose();

	bool isInitialized() const;

	bool setMesh(IGlMeshConstPtr mesh);
	inline IGlMeshConstPtr getMesh() const;

	bool computeRayIntersection(
		const KdTreeRaycastRequest& request,
		KdTreeRaycastResult& result) const;

private:
	class KdTreeMeshAccessor* m_meshAccessor= nullptr;
	class KdTreeData* m_treeData= nullptr;
};
