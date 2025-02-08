#pragma once

#include "MikanRendererFwd.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class MikanRenderModelResource
{
public:
	MikanRenderModelResource(class IMkWindow* ownerWindow);
	virtual ~MikanRenderModelResource();

	inline const std::string& getName() const { return m_name; }
	inline void setName(const std::string& inName) { m_name= inName; }

	inline const std::filesystem::path& getModelFilePath() const { return m_renderModelFilepath; }
	inline void setModelFilePath(const std::filesystem::path& inModelFilePath) 
	{ m_renderModelFilepath= inModelFilePath; }

	void addTriangulatedMesh(IMkTriangulatedMeshPtr mesh);
	void addWireframeMesh(IMkWireframeMeshPtr mesh);

	int getTriangulatedMeshCount() const 
	{ return (int)m_triangulatedMeshes.size(); }
	IMkTriangulatedMeshPtr getTriangulatedMesh(int meshIndex) const 
	{ return m_triangulatedMeshes[meshIndex]; }

	size_t getWireframeMeshCount() const { return m_wireframeMeshes.size(); }
	IMkWireframeMeshPtr getWireframeMesh(int meshIndex) const 
	{ return m_wireframeMeshes[meshIndex]; }

protected:
	void disposeMeshRenderResources();

	class IMkWindow* m_ownerWindow= nullptr;

	std::string m_name;
	std::filesystem::path m_renderModelFilepath;

	std::vector<IMkTriangulatedMeshPtr> m_triangulatedMeshes;
	std::vector<IMkWireframeMeshPtr> m_wireframeMeshes;
};