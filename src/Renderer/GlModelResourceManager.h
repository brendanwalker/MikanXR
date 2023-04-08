#pragma once

#include <filesystem>
#include <map>
#include <string>

#define PHONG_CAMERA_LOOKAT_UNIFORM_NAME				"cameraLookAt"
#define PHONG_CAMERA_PROJECTION_UNIFORM_NAME			"cameraProjection"
#define PHONG_CAMERA_POSITION_UNIFORM_NAME				"cameraPosition"

#define PHONG_MESH_TRANSFORM_UNIFORM_NAME				"meshTransform"
#define PHONG_INV_MESH_TRANSFORM_UNIFORM_NAME			"meshTransformTransposedInverse"

#define PHONG_AMBIENT_COLOR_UNIFORM_NAME				"ambientColor"
#define PHONG_DIFFUSE_COLOR_UNIFORM_NAME				"diffuseColor"
#define PHONG_SPECULAR_COLOR_UNIFORM_NAME				"specularColor"
#define PHONG_SHININESS_UNIFORM_NAME					"shininess"

#define PHONG_LIGHT_COLOR_UNIFORM_NAME					"lightColor"
#define PHONG_LIGHT_DIRECTION_UNIFORM_NAME				"lightDirection"

class GlRenderModelResource;
using GlRenderModelResourcePtr = std::shared_ptr<GlRenderModelResource>;
using GlRenderModelResourceWeakPtr = std::weak_ptr<GlRenderModelResource>;

class GlProgram;
using GlProgramPtr= std::shared_ptr<GlProgram>;

class GlMaterial;
using GlMaterialConstPtr= std::shared_ptr<const GlMaterial>;

class GlModelResourceManager
{
public:
	GlModelResourceManager();
	virtual ~GlModelResourceManager();

	static GlModelResourceManager* getInstance() { return s_modelResourceManager; }

	bool startup();
	void shutdown();

	GlRenderModelResourcePtr fetchRenderModel(
		const std::filesystem::path& modelFilePath,
		const struct GlVertexDefinition* vertexDefinition);

	GlProgramPtr getPhongShader() const { return m_phongShader; }
	GlMaterialConstPtr getPhongMaterial() const { return m_phongMaterial; }

private:
	static const class GlProgramCode* getPhongShaderCode();
	GlProgramPtr m_phongShader = nullptr;
	GlMaterialConstPtr m_phongMaterial = nullptr;

	std::map<std::string, GlRenderModelResourcePtr> m_renderModelCache;

	static GlModelResourceManager* s_modelResourceManager;
};
