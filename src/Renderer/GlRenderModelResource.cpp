#include "GlModelResourceManager.h"
#include "GlTriangulatedMesh.h"
#include "GlWireframeMesh.h"
#include "GlRenderModelResource.h"


GlRenderModelResource::GlRenderModelResource(IGlWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
}

GlRenderModelResource::~GlRenderModelResource()
{
	disposeMeshRenderResources();
}

void GlRenderModelResource::addTriangulatedMesh(GlTriangulatedMeshPtr mesh)
{
	if (mesh != nullptr)
	{
		m_triangulatedMeshes.push_back(mesh);
	}
}

void GlRenderModelResource::addWireframeMesh(GlWireframeMeshPtr mesh)
{
	if (mesh != nullptr)
	{
		m_wireframeMeshes.push_back(mesh);
	}
}

void GlRenderModelResource::replaceMeshes(GlRenderModelResourcePtr otherModelResource)
{
	disposeMeshRenderResources();

	for (GlTriangulatedMeshPtr triMesh : otherModelResource->m_triangulatedMeshes)
	{
		addTriangulatedMesh(triMesh);
	}

	for (GlWireframeMeshPtr wireframeMesh : otherModelResource->m_wireframeMeshes)
	{
		addWireframeMesh(wireframeMesh);
	}
}

void GlRenderModelResource::disposeMeshRenderResources()
{
	for (GlTriangulatedMeshPtr triMesh : m_triangulatedMeshes)
	{
		triMesh->deleteResources();
	}
	m_triangulatedMeshes.clear();

	for (GlWireframeMeshPtr wireframeMesh : m_wireframeMeshes)
	{
		wireframeMesh->deleteResources();
	}
	m_wireframeMeshes.clear();
}