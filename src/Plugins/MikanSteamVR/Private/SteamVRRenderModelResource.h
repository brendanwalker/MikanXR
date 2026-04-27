#pragma once

#include "MkRendererFwd.h"

#include <memory>
#include <string>

namespace vr
{
	struct RenderModel_t;
	struct RenderModel_TextureMap_t;
};

class SteamVRRenderModelResource
{
public:
	SteamVRRenderModelResource(class IMkGraphicsContext* ownerContext);
	virtual ~SteamVRRenderModelResource();

	// Phase 1: Load raw SteamVR model/texture data (safe to call on background thread)
	bool loadSteamVRResources();
	// Phase 2: Create GL textures and meshes from loaded data (must call on main/GL thread)
	bool createGraphicsResources();
	bool hasGraphicsResources() const { return m_mkDiffuseTexture != nullptr; }

	void disposeRenderResources();

	void setRenderModelName(const std::string& inRenderModelName) { m_renderModelName = inRenderModelName; }
	const std::string& getRenderModelName() const { return m_renderModelName; }
	MkMaterialInstancePtr getMaterial() const { return m_mkMaterialInstance; }
	const IMkTriangulatedMeshPtr getTriangulatedMesh() const { return m_mkTriangulatedMesh; }
	const IMkWireframeMeshPtr getWireframeMesh() const { return m_mkWireframeMesh; }

protected:
	void disposeSteamVRResources();

	IMkTexturePtr createTextureResource(
		const struct vr::RenderModel_TextureMap_t* steamvrTexture);
	MkMaterialInstancePtr createMaterialInstance(IMkTexturePtr texture);
	IMkTriangulatedMeshPtr createTriangulatedMeshResource(
		const std::string& meshName,
		MkMaterialInstancePtr materialInstance,
		const struct vr::RenderModel_t* steamVRRenderModel);
	IMkWireframeMeshPtr createWireframeMeshResource(
		const std::string& meshName,
		const vr::RenderModel_t* steamVRRenderModel);

	IMkGraphicsContext* m_ownerContext = nullptr;
	std::string m_renderModelName;

	vr::RenderModel_t* m_steamVRRenderModel = nullptr;
	vr::RenderModel_TextureMap_t* m_steamVRTextureMap = nullptr;

	IMkTriangulatedMeshPtr m_mkTriangulatedMesh = nullptr;
	IMkWireframeMeshPtr m_mkWireframeMesh = nullptr;
	IMkTexturePtr m_mkDiffuseTexture = nullptr;
	MkMaterialInstancePtr m_mkMaterialInstance = nullptr;
};