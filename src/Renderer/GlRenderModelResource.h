#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace objl
{
	class Loader;
	struct Mesh;
	struct Material;
};

class GlTriangulatedMesh;
using GlTriangulatedMeshPtr = std::shared_ptr<GlTriangulatedMesh>;

class GlWireframeMesh;
using GlWireframeMeshPtr = std::shared_ptr<GlWireframeMesh>;

class GlMaterialInstance;
using GlMaterialInstancePtr= std::shared_ptr<GlMaterialInstance>;

class GlRenderModelResource
{
public:
	GlRenderModelResource(const std::filesystem::path& modelFilePath);
	GlRenderModelResource(
		const std::filesystem::path& modelFilePath,
		const struct GlVertexDefinition* vertexDefinition);
	virtual ~GlRenderModelResource();

	bool createRenderResources();
	void disposeRenderResources();

	const std::filesystem::path& getRenderModelFilepath() const { return m_renderModelFilepath; }
	const GlVertexDefinition* getVertexDefinition() const { return m_vertexDefinition; }
	static const GlVertexDefinition* getDefaultVertexDefinition();

	size_t getTriangulatedMeshCount() const 
	{ return m_glTriMeshResources.size(); }
	GlTriangulatedMeshPtr getTriangulatedMesh(int meshIndex) const 
	{ return m_glTriMeshResources[meshIndex].glMesh; }
	GlMaterialInstancePtr getTriangulatedMeshMaterial(int meshIndex) const
	{ return m_glTriMeshResources[meshIndex].glMaterialInstance; }

	size_t getWireframeMeshCount() const { return m_glWireframeMeshResources.size(); }
	GlWireframeMeshPtr getWireframeMesh(int meshIndex) const 
	{ return m_glWireframeMeshResources[meshIndex].glMesh; }
	GlMaterialInstancePtr getWireframeMeshMaterial(int meshIndex) const
	{ return m_glWireframeMeshResources[meshIndex].glMaterialInstance; }

protected:
	bool loadObjFileResources();
	void disposeObjFileResources();

	GlTriangulatedMeshPtr createTriangulatedMeshResource(
		const std::string& meshName,
		const struct GlVertexDefinition* vertexDefinition,
		const objl::Mesh* objMesh);
	GlMaterialInstancePtr createTriMeshMaterialResource(
		const std::string& materialName,
		const objl::Material* objMaterial);
	GlWireframeMeshPtr createWireframeMeshResource(
		const std::string& meshName,
		const objl::Mesh* objMesh);
	GlMaterialInstancePtr createWireframeMeshMaterialResource(
		const std::string& materialName);

	objl::Loader* m_objLoader= nullptr;

	const std::filesystem::path m_renderModelFilepath;
	struct GlVertexDefinition* m_vertexDefinition= nullptr;

	struct TriMeshResourceEntry
	{
		GlTriangulatedMeshPtr glMesh;
		GlMaterialInstancePtr glMaterialInstance;
	};
	std::vector<TriMeshResourceEntry> m_glTriMeshResources;

	struct WireframeMeshResourceEntry
	{
		GlWireframeMeshPtr glMesh;
		GlMaterialInstancePtr glMaterialInstance;
	};
	std::vector<WireframeMeshResourceEntry> m_glWireframeMeshResources;
};
using GlRenderModelResourcePtr = std::shared_ptr<GlRenderModelResource>;
using GlRenderModelResourceWeakPtr = std::weak_ptr<GlRenderModelResource>;