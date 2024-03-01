#pragma once

#include "RendererFwd.h"

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
	SteamVRRenderModelResource(const std::string& renderModelName);
	virtual ~SteamVRRenderModelResource();

	bool createRenderResources();
	void disposeRenderResources();

	const std::string& getRenderModelName() const { return m_renderModelName; }
	GlMaterialInstancePtr getMaterial() const { return m_glMaterialInstance; }
	const GlTriangulatedMeshPtr getTriangulatedMesh() const { return m_glMesh; }

protected:
	bool loadSteamVRResources();
	void disposeSteamVRResources();

	GlTexturePtr createTextureResource(
		const struct vr::RenderModel_TextureMap_t* steamvrTexture);
	GlMaterialInstancePtr createMaterialInstance(GlTexturePtr texture);
	GlTriangulatedMeshPtr createTriangulatedMeshResource(
		const std::string& meshName,
		GlMaterialInstancePtr materialInstance,
		const struct vr::RenderModel_t* steamVRRenderModel);

	IGlWindow* m_ownerWindow= nullptr;
	std::string m_renderModelName;
	
	vr::RenderModel_t* m_steamVRRenderModel= nullptr;
	vr::RenderModel_TextureMap_t* m_steamVRTextureMap= nullptr;

	GlTriangulatedMeshPtr m_glMesh = nullptr;
	GlTexturePtr m_glDiffuseTexture = nullptr;
	GlMaterialInstancePtr m_glMaterialInstance= nullptr;
};
