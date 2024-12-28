#pragma once

#include "RendererFwd.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class GlRenderModelResource
{
public:
	GlRenderModelResource(class IGlWindow* ownerWindow);
	virtual ~GlRenderModelResource();

	inline const std::string& getName() const { return m_name; }
	inline void setName(const std::string& inName) { m_name= inName; }

	inline const std::filesystem::path& getModelFilePath() const { return m_renderModelFilepath; }
	inline void setModelFilePath(const std::filesystem::path& inModelFilePath) 
	{ m_renderModelFilepath= inModelFilePath; }

	void addTriangulatedMesh(GlTriangulatedMeshPtr mesh);
	void addWireframeMesh(GlWireframeMeshPtr mesh);

	int getTriangulatedMeshCount() const 
	{ return (int)m_triangulatedMeshes.size(); }
	GlTriangulatedMeshPtr getTriangulatedMesh(int meshIndex) const 
	{ return m_triangulatedMeshes[meshIndex]; }

	size_t getWireframeMeshCount() const { return m_wireframeMeshes.size(); }
	GlWireframeMeshPtr getWireframeMesh(int meshIndex) const 
	{ return m_wireframeMeshes[meshIndex]; }

protected:
	void disposeMeshRenderResources();

	class IGlWindow* m_ownerWindow= nullptr;

	std::string m_name;
	std::filesystem::path m_renderModelFilepath;

	std::vector<GlTriangulatedMeshPtr> m_triangulatedMeshes;
	std::vector<GlWireframeMeshPtr> m_wireframeMeshes;
};