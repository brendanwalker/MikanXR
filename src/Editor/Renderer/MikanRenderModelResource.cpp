#include "MikanModelResourceManager.h"
#include "IMkTriangulatedMesh.h"
#include "IMkWireframeMesh.h"
#include "MikanRenderModelResource.h"


MikanRenderModelResource::MikanRenderModelResource(IMkWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
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
		m_triangulatedMeshes.push_back(mesh);
	}
}

void MikanRenderModelResource::addWireframeMesh(IMkWireframeMeshPtr mesh)
{
	if (mesh != nullptr)
	{
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