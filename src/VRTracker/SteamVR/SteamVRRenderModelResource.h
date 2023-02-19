#pragma once

#include <memory>
#include <string>

namespace vr
{
	struct RenderModel_t;
	struct RenderModel_TextureMap_t;
};

class GlTexture;
typedef std::shared_ptr<GlTexture> GlTexturePtr; 

class GlMaterial;
typedef std::shared_ptr<GlMaterial> GlMaterialPtr;

class GlTriangulatedMesh;
typedef std::shared_ptr<GlTriangulatedMesh> GlTriangulatedMeshPtr;

class SteamVRRenderModelResource
{
public:
	SteamVRRenderModelResource(const std::string& renderModelName);
	virtual ~SteamVRRenderModelResource();

	bool createRenderResources();
	void disposeRenderResources();

	const std::string& getRenderModelName() const { return m_renderModelName; }
	GlMaterialPtr getMaterial() const { return m_glMaterial; }
	const GlTriangulatedMeshPtr getTriangulatedMesh() const { return m_glMesh; }

protected:
	bool loadSteamVRResources();
	void disposeSteamVRResources();

	static const class GlProgramCode* getShaderCode();
	static const struct GlVertexDefinition* getVertexDefinition();

	GlTexturePtr createTextureResource(
		const struct vr::RenderModel_TextureMap_t* steamvrTexture);
	GlMaterialPtr createMaterial(
		const class GlProgramCode* code, 
		GlTexturePtr texture);
	GlTriangulatedMeshPtr createTriangulatedMeshResource(
		const std::string& meshName,
		const struct GlVertexDefinition* vertexDefinition,
		const struct vr::RenderModel_t* steamVRRenderModel);

	std::string m_renderModelName;
	
	vr::RenderModel_t* m_steamVRRenderModel= nullptr;
	vr::RenderModel_TextureMap_t* m_steamVRTextureMap= nullptr;

	GlTriangulatedMeshPtr m_glMesh = nullptr;
	GlTexturePtr m_glDiffuseTexture = nullptr;
	GlMaterialPtr m_glMaterial= nullptr;
};
