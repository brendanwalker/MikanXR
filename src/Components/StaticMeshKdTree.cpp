#include "StaticMeshKdTree.h"
#include "ColliderQuery.h"
#include "IGlMesh.h"
#include "GlMaterialInstance.h"
#include "GlMaterial.h"
#include "GlProgram.h"
#include "MathUtility.h"
#include "MathGLM.h"

#include <glm/common.hpp>
#include <glm/gtx/intersect.hpp>

#include <algorithm>


class KdTreeNode
{
public:
	KdTreeNode() = default;
	KdTreeNode(int triangleIndex, const glm::vec3& minPoint, const glm::vec3& maxPoint)
		: m_triangleIndex(triangleIndex)
		, m_min(minPoint)
		, m_max(maxPoint)
	{}
	virtual ~KdTreeNode() 
	{
		if (m_left != nullptr)
			delete(m_left);
		if (m_right != nullptr)
			delete(m_right);
	}

	inline KdTreeNode* getLeft() const { return m_left; }
	inline KdTreeNode* getRight() const { return m_right; }
	inline int getTriangleIndex() const { return m_triangleIndex; }
	inline const glm::vec3& getMin() const { return m_min; }
	inline const glm::vec3& getMax() const { return m_max; }

	void setLeft(KdTreeNode* left) { m_left = left; }
	void setRight(KdTreeNode* right) { m_right = right; }
	void setTriangleIndex(int triangleIndex) { m_triangleIndex = triangleIndex; }
	void setMin(const glm::vec3& min) { m_min = min; }
	void setMax(const glm::vec3& max) { m_max = max; }

	void updateBoundingBox()
	{
		if (m_left != nullptr && m_right != nullptr)
		{
			m_min = glm::min(m_min, glm::min(m_left->m_min, m_right->m_min));
			m_max = glm::max(m_max, glm::max(m_left->m_max, m_right->m_max));
		}
		else if (m_left != nullptr)
		{
			m_min = glm::min(m_min, m_left->m_min);
			m_max = glm::max(m_max, m_left->m_max);
		}
		else if (m_right != nullptr)
		{
			m_min = glm::min(m_min, m_right->m_min);
			m_max = glm::max(m_max, m_right->m_max);
		}
	}

private:
	KdTreeNode* m_left = nullptr;
	KdTreeNode* m_right = nullptr;
	int m_triangleIndex = -1;
	glm::vec3 m_min = glm::vec3(0.f);
	glm::vec3 m_max = glm::vec3(0.f);
};

class KdTreeMeshAccessor
{
public :
	KdTreeMeshAccessor()= default;

	inline IGlMeshConstPtr getMesh() const { return m_mesh; }
	inline size_t getVertexCount() const { return m_vertexCount; }
	inline size_t getTriangleCount() const { return m_triangleCount; }
	inline bool isValid() const { return m_vertexCount > 0 && m_triangleCount > 0; }

	bool setMesh(IGlMeshConstPtr mesh)
	{
		if (mesh->getIndexPerElementCount() != 3)
			return false;

		auto material = mesh->getMaterialInstance()->getMaterial();
		auto vertexDefinition = material->getProgram()->getVertexDefinition();
		auto vertexAttrib = vertexDefinition.getFirstAttributeBySemantic(eVertexSemantic::position);
		if (!vertexAttrib)
			return false;

		const size_t triangleCount = mesh->getElementCount();
		if (triangleCount <= 0)
			return false;

		m_mesh= mesh;
		m_vertexCount = m_mesh->getVertexCount();
		m_vertexData = m_mesh->getVertexData();
		m_indexData = m_mesh->getIndexData();
		m_triangleCount = m_mesh->getElementCount();
		m_indexSize = m_mesh->getIndexSize();
		m_triangleStride = m_mesh->getIndexSize() * 3;
		m_vertexSize = vertexDefinition.getVertexSize();
		m_positionOffset = vertexAttrib->getOffset();

		return m_vertexCount > 0 && m_triangleCount > 0;
	}

	glm::vec3 extractPosition3f(uint32_t vertexIndex) const
	{
		assert (vertexIndex >= 0 && vertexIndex < m_vertexCount);
		const size_t positionDataOffset = (vertexIndex*m_vertexSize) + m_positionOffset;
			
		return *(const glm::vec3*)(m_vertexData + positionDataOffset);
	};

	void extractTriangleVertexIndices(
		uint32_t triangleIndex, 
		uint32_t &outI0, uint32_t &outI1, uint32_t &outI2) const
	{
		assert (triangleIndex >= 0 && triangleIndex < m_triangleCount);
		const uint8_t* triangleIndexData = (m_indexData + ((size_t)triangleIndex*m_triangleStride));

		assert(m_indexSize == 4 || m_indexSize == 2);
		if (m_indexSize == 2)
		{
			const uint16_t* indexPtrUint16 = (const uint16_t*)triangleIndexData;

			outI0 = (uint32_t)indexPtrUint16[0];
			outI1 = (uint32_t)indexPtrUint16[1];
			outI2 = (uint32_t)indexPtrUint16[2];
		}
		else
		{
			const uint32_t* indexPtrUint32 = (const uint32_t*)triangleIndexData;
				
			outI0 = indexPtrUint32[0];
			outI1 = indexPtrUint32[1];
			outI2 = indexPtrUint32[2];
		}
	}	

private:
	IGlMeshConstPtr m_mesh= nullptr;
	const uint8_t* m_vertexData = nullptr;
	const uint8_t* m_indexData = nullptr;
	size_t m_vertexCount = 0;
	size_t m_triangleCount = 0;
	size_t m_triangleStride = 0;
	size_t m_indexSize = 0;
	size_t m_vertexSize = 0;
	size_t m_positionOffset = 0;
};

namespace KdTree
{
	struct Triangle
	{
		uint32_t triangleIndex;
		glm::vec3 centroid;
	};

	bool buildTriangles(
		const KdTreeMeshAccessor* meshAccessor,
		std::vector<KdTree::Triangle>& outTriangles)
	{
		if (!meshAccessor->isValid())
			return false;

		for (size_t triIndex = 0; triIndex < meshAccessor->getTriangleCount(); ++triIndex)
		{
			uint32_t i0, i1, i2;
			meshAccessor->extractTriangleVertexIndices((uint32_t)triIndex, i0, i1, i2);

			const glm::vec3 v0 = meshAccessor->extractPosition3f(i0);
			const glm::vec3 v1 = meshAccessor->extractPosition3f(i1);
			const glm::vec3 v2 = meshAccessor->extractPosition3f(i2);

			Triangle tri;
			tri.triangleIndex = (uint32_t)triIndex;
			tri.centroid = (v0 + v1 + v2) / 3.f;
			outTriangles.push_back(tri);
		}

		return outTriangles.size() > 0;
	}

	KdTreeNode* buildKdTree(
		const KdTreeMeshAccessor* meshAccessor,
		std::vector<KdTree::Triangle>& triangles,
		size_t startIdx, size_t endIdx, int depth)
	{
		if (endIdx - startIdx <= 0)
			return nullptr;

		int axis = depth % 3;

		// Sorting by axis to insert
		if (axis == 0)
		{
			std::sort(
				triangles.begin() + startIdx,
				triangles.begin() + endIdx,
				[](const Triangle& t1, const Triangle& t2) {
				return t1.centroid.x < t2.centroid.x;
			});
		}
		else if (axis == 1)
		{
			std::sort(
				triangles.begin() + startIdx,
				triangles.begin() + endIdx,
				[](const Triangle& t1, const Triangle& t2) {
				return t1.centroid.y < t2.centroid.y;
			});
		}
		else if (axis == 2)
		{
			std::sort(
				triangles.begin() + startIdx,
				triangles.begin() + endIdx,
				[](const Triangle& t1, const Triangle& t2) {
				return t1.centroid.z < t2.centroid.z;
			});
		}

		size_t half = (startIdx + endIdx) / 2;
		uint32_t triangleIndex = triangles[half].triangleIndex;

		uint32_t i0, i1, i2;
		meshAccessor->extractTriangleVertexIndices(triangleIndex, i0, i1, i2);

		const glm::vec3 v0 = meshAccessor->extractPosition3f(i0);
		const glm::vec3 v1 = meshAccessor->extractPosition3f(i1);
		const glm::vec3 v2 = meshAccessor->extractPosition3f(i2);

		// Finding it bounding box
		const glm::vec3 minPoint = glm::min(glm::min(v0, v1), v2);
		const glm::vec3 maxPoint = glm::max(glm::max(v0, v1), v2);

		// Creating a node for this triangle
		KdTreeNode* median = new KdTreeNode(triangleIndex, minPoint, maxPoint);
		median->setLeft(buildKdTree(meshAccessor, triangles, startIdx, half, depth + 1));
		median->setRight(buildKdTree(meshAccessor, triangles, half + 1, endIdx, depth + 1));
		median->updateBoundingBox();

		return median;
	}

	void findClosestIntersection(
		const KdTreeRaycastRequest& request,
		const KdTreeMeshAccessor* meshAccessor,
		KdTreeNode* currentNode,
		KdTreeRaycastResult& result)
	{
		if (currentNode == nullptr)
			return;

		float aabbIntDistance;
		if (glm_intersect_aabb_with_ray(
				request.origin, request.direction, 
				currentNode->getMin(), currentNode->getMax(),
				aabbIntDistance))
		{
			uint32_t i0, i1, i2;
			meshAccessor->extractTriangleVertexIndices(currentNode->getTriangleIndex(), i0, i1, i2);

			GlmTriangle tri;
			tri.v0 = meshAccessor->extractPosition3f(i0);
			tri.v1 = meshAccessor->extractPosition3f(i1);
			tri.v2 = meshAccessor->extractPosition3f(i2);

			float intDistance;
			glm::vec3 intPoint;
			glm::vec3 intNormal;
			if (glm_intersect_tri_with_ray(
					tri,
					request.origin, request.direction,
					intDistance, intPoint, intNormal))
			{
				if (!result.hit || intDistance < result.distance)
				{
					result.hit = true;
					result.distance = intDistance;
					result.position = intPoint;
					result.normal = intNormal;
					result.triangleIndex = currentNode->getTriangleIndex();
				}
			}

			findClosestIntersection(request, meshAccessor, currentNode->getLeft(), result);
			findClosestIntersection(request, meshAccessor, currentNode->getLeft(), result);
		}
	}
};

StaticMeshKdTree::StaticMeshKdTree()
	: m_meshAccessor(new KdTreeMeshAccessor())
{
}

StaticMeshKdTree::~StaticMeshKdTree()
{
	dispose();

	if (m_meshAccessor != nullptr)
	{
		delete m_meshAccessor;
		m_meshAccessor = nullptr;
	}
}

bool StaticMeshKdTree::isInitialized() const
{ 
	return m_meshAccessor->isValid();
}

IGlMeshConstPtr StaticMeshKdTree::getMesh() const
{ 
	return m_meshAccessor->getMesh(); 
}

bool StaticMeshKdTree::setMesh(IGlMeshConstPtr mesh)
{
	if (m_meshAccessor->getMesh() != mesh)
	{
		dispose();
	}

	return m_meshAccessor->setMesh(mesh);
}

bool StaticMeshKdTree::init()
{
	dispose();

	std::vector<KdTree::Triangle> m_triangles;
	if (KdTree::buildTriangles(m_meshAccessor, m_triangles))
	{
		m_root = buildKdTree(m_meshAccessor, m_triangles, 0, m_triangles.size(), 0);
	}

	return m_root != nullptr;
}

void StaticMeshKdTree::dispose()
{
	if (m_root != nullptr)
	{
		delete m_root;
		m_root = nullptr;
	}
}

bool StaticMeshKdTree::computeRayIntersection(
	const KdTreeRaycastRequest& request,
	KdTreeRaycastResult& result) const
{
	KdTree::findClosestIntersection(request, m_meshAccessor, m_root, result);

	return result.hit;
}