#pragma once

#include <string>
#include <vector>

namespace objl
{
	class Loader;
	struct Mesh;
};

class GlRenderModelResource
{
public:
	GlRenderModelResource(
		const std::string& renderModelFilepath,
		const struct GlVertexDefinition* vertexDefinition);
	virtual ~GlRenderModelResource();

	bool createRenderResources();
	void disposeRenderResources();

	const std::string& getRenderModelFilepath() const { return m_renderModelFilepath; }
	const GlVertexDefinition* getVertexDefinition() const { return m_vertexDefinition; }

	size_t getTriangulatedMeshCount() const { return m_glMeshes.size(); }
	const class GlTriangulatedMesh* getTriangulatedMesh(int meshIndex) const { return m_glMeshes[meshIndex]; }

	size_t getWireframeMeshCount() const { return m_glWireframeMeshes.size(); }
	const class GlWireframeMesh* getWireframeMesh(int meshIndex) const { return m_glWireframeMeshes[meshIndex]; }

protected:
	bool loadObjFileResources();
	void disposeObjFileResources();

	class GlTriangulatedMesh* createTriangulatedMeshResource(
		const std::string& meshName,
		const struct GlVertexDefinition* vertexDefinition,
		const objl::Mesh* objMesh);
	class GlWireframeMesh* createWireframeMeshResource(
		const std::string& meshName,
		const objl::Mesh* objMesh);

	objl::Loader* m_objLoader= nullptr;

	std::string m_renderModelFilepath;
	struct GlVertexDefinition* m_vertexDefinition= nullptr;
	std::vector<class GlTriangulatedMesh*> m_glMeshes;
	std::vector<class GlWireframeMesh*> m_glWireframeMeshes;
};
