#pragma once

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
	const class GlMaterial* getMaterial() const { return m_glMaterial; }
	const class GlTriangulatedMesh* getTriangulatedMesh() const { return m_glMesh; }

protected:
	bool loadSteamVRResources();
	void disposeSteamVRResources();

	static const class GlProgramCode* getShaderCode();
	static const struct GlVertexDefinition* getVertexDefinition();

	class GlTexture* createTextureResource(
		const struct vr::RenderModel_TextureMap_t* steamvrTexture);
	class GlMaterial* createMaterial(
		const class GlProgramCode* code, 
		class GlTexture* texture);
	class GlTriangulatedMesh* createTriangulatedMeshResource(
		const std::string& meshName,
		const struct GlVertexDefinition* vertexDefinition,
		const struct vr::RenderModel_t* steamVRRenderModel);

	std::string m_renderModelName;
	
	vr::RenderModel_t* m_steamVRRenderModel= nullptr;
	vr::RenderModel_TextureMap_t* m_steamVRTextureMap= nullptr;

	class GlTriangulatedMesh* m_glMesh = nullptr;
	class GlTexture* m_glDiffuseTexture = nullptr;
	class GlMaterial *m_glMaterial= nullptr;
};
