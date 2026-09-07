#pragma once

#include "MikanRendererFwd.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class MikanRenderModelResource
{
public:
	MikanRenderModelResource(class IMkGraphicsContext* ownerGraphicsContext);
	virtual ~MikanRenderModelResource();

	inline const std::string& getName() const { return m_name; }
	inline void setName(const std::string& inName) { m_name= inName; }

	inline const std::filesystem::path& getModelFilePath() const { return m_renderModelFilepath; }
	inline void setModelFilePath(const std::filesystem::path& inModelFilePath)
	{
		m_renderModelFilepath= inModelFilePath;
	}

	void addTriangulatedMesh(IMkTriangulatedMeshPtr mesh);
	void addWireframeMesh(IMkWireframeMeshPtr mesh);

	int getTriangulatedMeshCount() const { return (int)m_triangulatedMeshes.size(); }
	IMkTriangulatedMeshPtr getTriangulatedMesh(int meshIndex) const { return m_triangulatedMeshes[meshIndex]; }

	size_t getWireframeMeshCount() const { return m_wireframeMeshes.size(); }
	IMkWireframeMeshPtr getWireframeMesh(int meshIndex) const { return m_wireframeMeshes[meshIndex]; }

	// Serialized render geometry for clients that ask for this model's triangle data. Built on the
	// first request and reused after, since the payload is a pure function of the loaded meshes and
	// those never change for the life of the resource. Held as opaque bytes so the renderer stays
	// free of client API types; the server side owns what the bytes mean.
	inline const std::vector<uint8_t>& getClientGeometryPayload() const { return m_clientGeometryPayload; }
	inline void setClientGeometryPayload(std::vector<uint8_t>&& payload)
	{
		m_clientGeometryPayload= std::move(payload);
	}

protected:
	void disposeMeshRenderResources();

	class IMkGraphicsContext* m_ownerGraphicsContext= nullptr;

	std::string m_name;
	std::filesystem::path m_renderModelFilepath;

	std::vector<IMkTriangulatedMeshPtr> m_triangulatedMeshes;
	std::vector<IMkWireframeMeshPtr> m_wireframeMeshes;
	std::vector<uint8_t> m_clientGeometryPayload;
};