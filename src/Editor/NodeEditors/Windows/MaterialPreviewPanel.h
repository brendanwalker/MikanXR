#pragma once

#include "MkRendererFwd.h"
#include "MikanRendererFwd.h"
#include "MaterialCompiler/MaterialCompileResult.h"
#include "MaterialCompiler/MaterialDomain.h"

#include <map>
#include <string>
#include <vector>

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"

// The material editor's live picture: compiles the last MaterialCompileResult
// into its own shader program and draws a preset-matched mesh with it into an
// owned framebuffer every frame. All GL work happens in render(), where the
// owning window's context is current; setCompileResult only parks the result.
class MaterialPreviewPanel
{
public:
	MaterialPreviewPanel()= default;

	// Park a compile result for the next render. A result with graph errors
	// leaves the previous good material on screen.
	void setCompileResult(const MaterialCompileResult& result, eMaterialDomain domain, eMaterialVertexPreset preset);

	// Draw the preview into the owned framebuffer (call with the window's GL context current)
	void render(IMkGraphicsContext* graphicsContext, MikanTextureCache* textureCache,
				MikanModelResourceManager* modelResourceManager, float deltaSeconds);

	// Draw the framebuffer's color texture and any status text into the current ImGui window
	void renderUi(class MkGuiStyleManager* styleManager);

	// Release the GL resources (call from the window's shutdown)
	void dispose();

	const std::string& getCompileLog() const { return m_compileLog; }

private:
	bool compilePendingResult(IMkGraphicsContext* graphicsContext, MikanModelResourceManager* modelResourceManager);
	bool rebuildMeshes(IMkGraphicsContext* graphicsContext, MikanModelResourceManager* modelResourceManager);
	bool buildUnitQuadMesh(IMkGraphicsContext* graphicsContext);
	bool buildSphereMeshes(IMkGraphicsContext* graphicsContext, MikanModelResourceManager* modelResourceManager);
	bool ensureFrameBuffer();
	void refreshParameterTextures(MikanTextureCache* textureCache);
	void updateOrbitCamera(float deltaSeconds);
	void drawPreview(IMkGraphicsContext* graphicsContext, MikanTextureCache* textureCache);

	// The parked compile result, consumed by the next render
	MaterialCompileResult m_pendingResult;
	eMaterialDomain m_pendingDomain= eMaterialDomain::INVALID;
	eMaterialVertexPreset m_pendingPreset= eMaterialVertexPreset::INVALID;
	bool m_bHasPendingResult= false;

	// The last good compile
	eMaterialDomain m_domain= eMaterialDomain::INVALID;
	eMaterialVertexPreset m_preset= eMaterialVertexPreset::INVALID;
	IMkShaderPtr m_shader;
	MkMaterialPtr m_material;
	MkMaterialInstancePtr m_materialInstance;
	std::vector<MaterialParameterDefault> m_parameterDefaults;
	std::map<std::string, IMkTexturePtr> m_parameterTextures;
	bool m_bParameterTexturesDirty= false;

	// Preview geometry for the current preset
	std::vector<IMkTriangulatedMeshPtr> m_meshes;
	MikanRenderModelResourcePtr m_sphereModel;
	bool m_bLoggedSphereFallback= false;

	IMkFrameBufferPtr m_frameBuffer;

	// Orbit camera state for the shape domain
	float m_orbitAngleRadians= 0.f;
	float m_accumulatedSeconds= 0.f;
	glm::vec3 m_cameraPosition= glm::vec3(0.f);
	glm::mat4 m_viewMatrix= glm::mat4(1.f);
	glm::mat4 m_projectionMatrix= glm::mat4(1.f);

	// Driver info log of the last failed compile, empty after a success
	std::string m_compileLog;
	// Uniforms the last draw left unbound, empty when the draw bound everything
	std::string m_unboundUniformStatus;
};
