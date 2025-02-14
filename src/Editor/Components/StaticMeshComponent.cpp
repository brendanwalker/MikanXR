#include "MikanModelResourceManager.h"
#include "MkMaterialInstance.h"
#include "MkMaterial.h"
#include "IMkShader.h"
#include "IMkStaticMeshInstance.h"
#include "IMkVertexDefinition.h"
#include "StaticMeshComponent.h"
#include "MikanObject.h"
#include "MikanStencilTypes.h"

StaticMeshComponent::StaticMeshComponent(MikanObjectWeakPtr owner)
	: SceneComponent(owner)
{
}

IMkStaticMeshInstancePtr StaticMeshComponent::getStaticMesh() const 
{ 
	return std::dynamic_pointer_cast<IMkStaticMeshInstance>(m_sceneRenderable); 
}

void StaticMeshComponent::setStaticMesh(IMkStaticMeshInstancePtr meshInstance)
{
	m_sceneRenderable= meshInstance;
	if (OnMeshChanged)
	{
		StaticMeshComponentWeakPtr meshComponent= getSelfWeakPtr<StaticMeshComponent>();

		OnMeshChanged(meshComponent);
	}
}

void StaticMeshComponent::extractRenderGeometry(MikanTriagulatedMesh& outRenderGeometry)
{
	IMkStaticMeshInstancePtr meshInstance= getStaticMesh();

	IMkMeshConstPtr glMesh = meshInstance->getMesh();
	if (glMesh->getIndexPerElementCount() != 3)
	{
		// Only support triangle meshes
		return;
	}

	MkMaterialInstanceConstPtr materialInst= meshInstance->getMaterialInstanceConst();
	MkMaterialConstPtr material= materialInst->getMaterial();
	IMkVertexDefinitionConstPtr vertexDefinition= material->getProgram()->getVertexDefinition();
	const size_t vertexSize = vertexDefinition->getVertexSize();
	const uint32_t vertexCount = glMesh->getVertexCount();
	const uint8_t* vertexData = glMesh->getVertexData();

	// Write out the vertices
	const IMkVertexAttribute* posAttrib= vertexDefinition->getFirstAttributeBySemantic(eVertexSemantic::position);
	if (posAttrib != nullptr)
	{
		const uint8_t* posData = vertexData + posAttrib->getOffset();
		for (uint32_t i = 0; i < vertexCount; i++)
		{
			const MikanVector3f& pos = *(const MikanVector3f*)posData;

			outRenderGeometry.vertices.push_back(pos);
			posData += vertexSize;
		}
	}

	// Write out the normals
	const IMkVertexAttribute* normalAttrib= vertexDefinition->getFirstAttributeBySemantic(eVertexSemantic::normal);
	if (normalAttrib != nullptr)
	{
		const uint8_t* normalData = vertexData + normalAttrib->getOffset();
		for (uint32_t i = 0; i < vertexCount; i++)
		{
			const MikanVector3f& normal = *(const MikanVector3f*)normalData;

			outRenderGeometry.normals.push_back(normal);
			normalData += vertexSize;
		}
	}

	// Write out the texture coordinates
	const IMkVertexAttribute* texCoordAttrib= vertexDefinition->getFirstAttributeBySemantic(eVertexSemantic::texCoord);
	if (texCoordAttrib != nullptr)
	{
		const uint8_t* texCoordData = vertexData + texCoordAttrib->getOffset();
		for (uint32_t i = 0; i < vertexCount; i++)
		{
			const MikanVector2f& texel = *(const MikanVector2f*)texCoordData;

			outRenderGeometry.texels.push_back(texel);
			texCoordData += vertexSize;
		}
	}

	// Write out the indices
	const uint32_t indexCount = (uint32_t)(glMesh->getElementCount() * glMesh->getIndexPerElementCount());
	const uint8_t* indexData = glMesh->getIndexData();
	const size_t indexSize = glMesh->getIndexSize();
	if (indexSize == sizeof(uint16_t))
	{
		for (uint32_t i = 0; i < indexCount; i++)
		{
			const uint16_t index = *(const uint16_t*)indexData;

			outRenderGeometry.indices.push_back((int)index);
			indexData += indexSize;
		}
	}
	else
	{
		for (uint32_t i = 0; i < indexCount; i++)
		{
			const uint32_t index = *(const uint32_t*)indexData;

			outRenderGeometry.indices.push_back((int)index);
			indexData += indexSize;
		}
	}
}