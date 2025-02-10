#pragma once

#include "MikanRendererFwd.h"

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
	SteamVRRenderModelResource(class IMkWindow* ownerWindow);
	virtual ~SteamVRRenderModelResource();

	bool createRenderResources();
	void disposeRenderResources();

	void setRenderModelName(const std::string& inRenderModelName) { m_renderModelName= inRenderModelName; }
	const std::string& getRenderModelName() const { return m_renderModelName; }
	MkMaterialInstancePtr getMaterial() const { return m_glMaterialInstance; }
	const IMkTriangulatedMeshPtr getTriangulatedMesh() const { return m_glMesh; }

protected:
	bool loadSteamVRResources();
	void disposeSteamVRResources();

	IMkTexturePtr createTextureResource(
		const struct vr::RenderModel_TextureMap_t* steamvrTexture);
	MkMaterialInstancePtr createMaterialInstance(IMkTexturePtr texture);
	IMkTriangulatedMeshPtr createTriangulatedMeshResource(
		const std::string& meshName,
		MkMaterialInstancePtr materialInstance,
		const struct vr::RenderModel_t* steamVRRenderModel);

	IMkWindow* m_ownerWindow= nullptr;
	std::string m_renderModelName;
	
	vr::RenderModel_t* m_steamVRRenderModel= nullptr;
	vr::RenderModel_TextureMap_t* m_steamVRTextureMap= nullptr;

	IMkTriangulatedMeshPtr m_glMesh = nullptr;
	IMkTexturePtr m_glDiffuseTexture = nullptr;
	MkMaterialInstancePtr m_glMaterialInstance= nullptr;
};
