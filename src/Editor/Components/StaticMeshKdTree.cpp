#include "StaticMeshKdTree.h"
#include "ColliderQuery.h"
#include "Colors.h"
#include "IMkMesh.h"
#include "MikanLineRenderer.h"
#include "MkMaterialInstance.h"
#include "MkMaterial.h"
#include "IMkShader.h"
#include "MathUtility.h"
#include "MathGLM.h"

#include <glm/common.hpp>
#include <glm/gtx/intersect.hpp>

#include <algorithm>

// -- KdTreeNode -----
class KdTreeNode
{
public:
	KdTreeNode() = default;
	KdTreeNode(int32_t triangleIndex, const glm::vec3& minPoint, const glm::vec3& maxPoint)
		: m_triangleIndex(triangleIndex)
		, m_min(minPoint)
		, m_max(maxPoint)
	{}

	inline int32_t getLeftNodeIndex() const { return m_leftNodeIndex; }
	inline int32_t getRightNodeIndex() const { return m_rightNodeIndex; }
	inline int32_t getTriangleIndex() const { return m_triangleIndex; }
	inline const glm::vec3& getMin() const { return m_min; }
	inline const glm::vec3& getMax() const { return m_max; }

	void setLeft(int32_t nodeIndex) { m_leftNodeIndex = nodeIndex; }
	void setRight(int32_t nodeIndex) { m_rightNodeIndex = nodeIndex; }

	void updateBoundingBox(const KdTreeNode* left, const KdTreeNode* right)
	{
		if (left != nullptr && right != nullptr)
		{
			m_min = glm::min(m_min, glm::min(left->m_min, right->m_min));
			m_max = glm::max(m_max, glm::max(left->m_max, right->m_max));
		}
		else if (m_leftNodeIndex != -1)
		{
			m_min = glm::min(m_min, left->m_min);
			m_max = glm::max(m_max, left->m_max);
		}
		else if (m_rightNodeIndex != -1)
		{
			m_min = glm::min(m_min, right->m_min);
			m_max = glm::max(m_max, right->m_max);
		}
	}

private:
	glm::vec3 m_min = glm::vec3(0.f);
	glm::vec3 m_max = glm::vec3(0.f);
	int32_t m_leftNodeIndex = -1;
	int32_t m_rightNodeIndex = -1;
	int32_t m_triangleIndex = -1;
};

// -- KdTreeData -----
class KdTreeData
{
public:
	KdTreeData() = default;
	KdTreeData(int32_t initialCapaity) 
		: m_nodes(new KdTreeNode[initialCapaity])
		, m_nodeCapacity(initialCapaity)
		, m_nodeCount(0)
	{ }

	~KdTreeData()
	{
		if (m_nodes != nullptr)
		{
			delete[] m_nodes;
			m_nodes = nullptr;
		}
	}

	int32_t getNodeCount() const { return m_nodeCount; }
	KdTreeNode* getNode(int32_t nodeIndex) const
	{
		if (nodeIndex >= 0 && nodeIndex < m_nodeCount)
		{
			return &m_nodes[nodeIndex];
		}

		return nullptr;
	}

	KdTreeNode* allocateNode(
		const int32_t triangleIndex, 
		const glm::vec3& minPoint, const glm::vec3& maxPoint,
		int32_t& outNodeIndex)
	{
		assert(m_nodeCount < m_nodeCapacity);
		if (m_nodeCount < m_nodeCapacity)
		{
			int32_t newIndex = m_nodeCount;
		 	KdTreeNode* newNode = new(&m_nodes[newIndex]) KdTreeNode(triangleIndex, minPoint, maxPoint);
			m_nodeCount++;

			outNodeIndex= newIndex;
			return newNode;
		}

		outNodeIndex= -1;
		return nullptr;
	}

protected:
	KdTreeNode* m_nodes = nullptr;
	int32_t m_nodeCapacity = 0;
	int32_t m_nodeCount = 0;
};

// -- KdTreeMeshAccessor -----
class KdTreeMeshAccessor
{
public :
	KdTreeMeshAccessor()= default;

	inline IMkMeshConstPtr getMesh() const { return m_mesh; }
	inline size_t getVertexCount() const { return m_vertexCount; }
	inline size_t getTriangleCount() const { return m_triangleCount; }
	inline bool isValid() const { return m_vertexCount > 0 && m_triangleCount > 0; }

	bool setMesh(IMkMeshConstPtr mesh)
	{
		if (mesh->getIndexPerElementCount() != 3)
			return false;

		auto material = mesh->getMaterialInstance()->getMaterial();
		auto vertexDefinition = material->getProgram()->getVertexDefinition();
		auto vertexAttrib = vertexDefinition->getFirstAttributeBySemantic(eVertexSemantic::position);
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
		m_vertexSize = vertexDefinition->getVertexSize();
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
	IMkMeshConstPtr m_mesh= nullptr;
	const uint8_t* m_vertexData = nullptr;
	const uint8_t* m_indexData = nullptr;
	size_t m_vertexCount = 0;
	size_t m_triangleCount = 0;
	size_t m_triangleStride = 0;
	size_t m_indexSize = 0;
	size_t m_vertexSize = 0;
	size_t m_positionOffset = 0;
};

// -- KdTree Utilities -----
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

	int32_t buildKdTree(
		const KdTreeMeshAccessor* meshAccessor,
		KdTreeData* treeData,
		std::vector<KdTree::Triangle>& triangles,
		size_t startIdx, size_t endIdx, int depth)
	{
		if (endIdx - startIdx <= 0)
			return -1;

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
		int32_t medianNodeIndex= -1;
		KdTreeNode* median = treeData->allocateNode(triangleIndex, minPoint, maxPoint, medianNodeIndex);
		assert(median != nullptr);
		if (median != nullptr)
		{
			median->setLeft(buildKdTree(meshAccessor, treeData, triangles, startIdx, half, depth + 1));
			median->setRight(buildKdTree(meshAccessor, treeData, triangles, half + 1, endIdx, depth + 1));

			// Update the bounding box of the median node to encompass its children
			{
				const KdTreeNode* left = treeData->getNode(median->getLeftNodeIndex());
				const KdTreeNode* right = treeData->getNode(median->getRightNodeIndex());

				median->updateBoundingBox(left, right);
			}
		}

		return medianNodeIndex;
	}

	void findClosestIntersection(
		const KdTreeRaycastRequest& request,
		const KdTreeMeshAccessor* meshAccessor,
		KdTreeData* treeData,
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
			if (request.debugDraw)
			{
				drawTransformedBox(
					request.worldMatrix, 
					currentNode->getMin(), currentNode->getMax(), 
					Colors::Red);
			}

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
				if (request.debugDraw)
				{
					drawTransformedTriangle(request.worldMatrix, tri, Colors::Yellow);
				}

				if (!result.hit || intDistance < result.distance)
				{
					result.hit = true;
					result.distance = intDistance;
					result.position = intPoint;
					result.normal = intNormal;
					result.triangleIndex = currentNode->getTriangleIndex();
				}
			}
			
			KdTreeNode* leftNode= treeData->getNode(currentNode->getLeftNodeIndex());
			findClosestIntersection(request, meshAccessor, treeData, leftNode, result);

			KdTreeNode* rightNode = treeData->getNode(currentNode->getRightNodeIndex());
			findClosestIntersection(request, meshAccessor, treeData, rightNode, result);
		}
	}
};

// -- StaticMeshKdTree -----
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

IMkMeshConstPtr StaticMeshKdTree::getMesh() const
{ 
	return m_meshAccessor->getMesh(); 
}

bool StaticMeshKdTree::setMesh(IMkMeshConstPtr mesh)
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
		m_treeData = new KdTreeData((int32_t)m_triangles.size());
		buildKdTree(m_meshAccessor, m_treeData, m_triangles, 0, m_triangles.size(), 0);
	}

	return m_treeData != nullptr;
}

void StaticMeshKdTree::dispose()
{
	if (m_treeData != nullptr)
	{
		delete m_treeData;
		m_treeData= nullptr;
	}
}

bool StaticMeshKdTree::getLocalAABB(
	glm::vec3& outMin,
	glm::vec3& outMax) const
{
	if (m_treeData != nullptr && m_treeData->getNodeCount() > 0)
	{
		KdTreeNode* rootNode= m_treeData->getNode(0);

		outMin= rootNode->getMin();
		outMax= rootNode->getMax();

		return true;
	}

	return false;
}

bool StaticMeshKdTree::computeRayIntersection(
	const KdTreeRaycastRequest& request,
	KdTreeRaycastResult& result) const
{
	KdTree::findClosestIntersection(request, m_meshAccessor, m_treeData, m_treeData->getNode(0), result);

	return result.hit;
}

bool StaticMeshKdTree::computeClosestVertex(
	const glm::vec3& localPoint,
	const int triangleIndex,
	glm::vec3& closestVertex) const
{
	if (m_meshAccessor != nullptr &&
		triangleIndex > 0 && 
		triangleIndex < m_meshAccessor->getTriangleCount())
	{
		uint32_t i0, i1, i2;
		m_meshAccessor->extractTriangleVertexIndices(triangleIndex, i0, i1, i2);

		glm::vec3 vertices[3] = {
			m_meshAccessor->extractPosition3f(i0),
			m_meshAccessor->extractPosition3f(i1),
			m_meshAccessor->extractPosition3f(i2)
		};

		float closestDist = FLT_MAX;
		for (int i = 0; i < 3; ++i)
		{
			float dist = glm::distance(localPoint, vertices[i]);
			if (dist < closestDist)
			{
				closestDist = dist;
				closestVertex = vertices[i];
			}
		}

		return closestDist < FLT_MAX;
	}

	return false;
}