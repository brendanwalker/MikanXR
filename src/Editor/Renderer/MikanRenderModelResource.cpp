#include "MikanModelResourceManager.h"
#include "IMkTriangulatedMesh.h"
#include "IMkWireframeMesh.h"
#include "MikanRenderModelResource.h"


MikanRenderModelResource::MikanRenderModelResource(IMkGraphicsContext* ownerGraphicsContext)
	: m_ownerGraphicsContext(ownerGraphicsContext)
{
}

MikanRenderModelResource::~MikanRenderModelResource()
{
	disposeMeshRenderResources();
}

void MikanRenderModelResource::addTriangulatedMesh(IMkTriangulatedMeshPtr mesh)
{
	if (mesh != nullptr)
	{
		assert(mesh->getOwnerContext() == m_ownerGraphicsContext);
		m_triangulatedMeshes.push_back(mesh);
	}
}

void MikanRenderModelResource::addWireframeMesh(IMkWireframeMeshPtr mesh)
{
	if (mesh != nullptr)
	{
		assert(mesh->getOwnerContext() == m_ownerGraphicsContext);
		m_wireframeMeshes.push_back(mesh);
	}
}

void MikanRenderModelResource::disposeMeshRenderResources()
{
	for (IMkTriangulatedMeshPtr triMesh : m_triangulatedMeshes)
	{
		triMesh->deleteResources();
	}
	m_triangulatedMeshes.clear();

	for (IMkWireframeMeshPtr wireframeMesh : m_wireframeMeshes)
	{
		wireframeMesh->deleteResources();
	}
	m_wireframeMeshes.clear();
}