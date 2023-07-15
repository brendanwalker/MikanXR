#include "GlMaterial.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlVertexDefinition.h"
#include "Logger.h"

GlModelResourceManager* GlModelResourceManager::s_modelResourceManager= nullptr;

GlModelResourceManager::GlModelResourceManager()
{
	s_modelResourceManager= this;
}

GlModelResourceManager::~GlModelResourceManager()
{
	shutdown();
	s_modelResourceManager= nullptr;
}

bool GlModelResourceManager::startup()
{
	m_phongShader = GlShaderCache::getInstance()->fetchCompiledGlProgram(getPhongShaderCode());
	if (m_phongShader != nullptr)
	{
		m_phongMaterial = std::make_shared<GlMaterial>("Blinn-Phong BRDF", m_phongShader);
	}
	else
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile phong shader";
		return false;
	}

	m_wireframeShader = GlShaderCache::getInstance()->fetchCompiledGlProgram(getWireframeShaderCode());
	if (m_wireframeShader != nullptr)
	{
		m_wireframeMaterial = std::make_shared<GlMaterial>("Simple Wireframe", m_wireframeShader);
	}
	else
	{
		MIKAN_LOG_ERROR("GlFrameCompositor::startup()") << "Failed to compile wireframe shader";
		return false;
	}

	return true;
}

void GlModelResourceManager::shutdown()
{
	for (auto it = m_renderModelCache.begin(); it != m_renderModelCache.end(); ++it)
	{
		GlRenderModelResourcePtr resource = it->second;

		resource->disposeRenderResources();
	}

	m_renderModelCache.clear();
}

GlRenderModelResourcePtr GlModelResourceManager::fetchRenderModel(
	const std::filesystem::path& modelFilePath,
	const GlVertexDefinition* vertexDefinition)
{
	if (!modelFilePath.empty())
	{
		std::string hashName = modelFilePath.string() + vertexDefinition->getVertexDefinitionDesc();

		if (m_renderModelCache.find(hashName) != m_renderModelCache.end())
		{
			return m_renderModelCache[hashName];
		}
		else
		{
			GlRenderModelResourcePtr resource = std::make_shared<GlRenderModelResource>(modelFilePath, vertexDefinition);

			if (resource->createRenderResources())
			{
				m_renderModelCache[hashName] = resource;

				return resource;
			}
			else
			{
				return nullptr;
			}
		}
	}

	return nullptr;
}

const GlProgramCode* GlModelResourceManager::getPhongShaderCode()
{
	// https://www.gsn-lib.org/docs/nodes/ShaderPluginNode.php
	static GlProgramCode x_shaderCode = GlProgramCode(
		"Internal Phong Shader Code",
		// vertex shader
		R""""(
			#version 330 core
			layout (location = 0) in vec3 position;
			layout (location = 1) in vec3 normal;
			layout (location = 2) in vec2 texcoord;

			uniform mat4 cameraLookAt; //camera look at matrix
			uniform mat4 cameraProjection; //camera projection matrix
			uniform mat4 meshTransform; // mesh transformation
			uniform mat4 meshTransformTransposedInverse; // transposed inverse of meshTransform

			out vec2 tc; // output texture coordinate of vertex
			out vec3 wfn; // output fragment normal of vertex in world coordinate system
			out vec3 vertPos; // output 3D position in world coordinate system

			void main()
			{
				tc = texcoord;
				wfn = vec3(meshTransformTransposedInverse * vec4(normal, 0.0));
				vec4 vertPos4 = meshTransform * vec4(position, 1.0);
				vertPos = vec3(vertPos4) / vertPos4.w;
				gl_Position = cameraProjection * cameraLookAt * vertPos4;
			}
			)"""",
		//fragment shader
		R""""(
			#version 330 core
			out vec4 outColor;

			in vec2 tc; // texture coordinate of pixel (interpolated)
			in vec3 wfn; // fragment normal of pixel (interpolated)
			in vec3 vertPos; // fragment vertex position (interpolated)

			uniform vec4 ambientColor; // description="ambient color" defaultval="0.05, 0.0, 0.0, 1.0"
			uniform vec4 diffuseColor; // description="diffuse color" defaultval="0.2, 0.0, 0.0, 1.0"
			uniform vec4 specularColor; // description="specular color" defaultval="1.0, 1.0, 1.0, 1.0"
			uniform float shininess; // description="specular shininess exponent" defaultval="20.0"
			uniform vec4 lightColor; // description="color of light" defaultval="1.0, 1.0, 1.0, 1.0"
			uniform vec3 lightDirection; // light direction in world space
			uniform vec3 cameraPosition; // camera position in world space

			const float irradiPerp = 1.0;

			vec3 rgb2lin(vec3 rgb) { // sRGB to linear approximation
			  return pow(rgb, vec3(2.2));
			}

			vec3 lin2rgb(vec3 lin) { // linear to sRGB approximation
			  return pow(lin, vec3(1.0 / 2.2));
			}

			vec3 blinnPhongBRDF(vec3 lightDir, vec3 viewDir, vec3 normal, 
						vec3 phongDiffuseCol, vec3 phongSpecularCol, float phongShininess) {
			  vec3 color = phongDiffuseCol;
			  vec3 halfDir = normalize(viewDir + lightDir);
			  float specDot = max(dot(halfDir, normal), 0.0);
			  color += pow(specDot, phongShininess) * phongSpecularCol;
			  return color;
			}

			void main() {
			  vec3 lightDir = normalize(-lightDirection); // towards light
			  vec3 viewDir = normalize(cameraPosition - vertPos);
			  vec3 n = normalize(wfn);

			  vec3 radiance = rgb2lin(ambientColor.rgb);
  
			  // irradiance contribution from light
			  float irradiance = max(dot(lightDir, n), 0.0) * irradiPerp; 
			  if(irradiance > 0.0) { // if receives light
				vec3 brdf = blinnPhongBRDF(lightDir, viewDir, n, rgb2lin(diffuseColor.rgb), 
										   rgb2lin(specularColor.rgb), shininess);
				radiance += brdf * irradiance * lightColor.rgb;
			  }

			  outColor.rgb = lin2rgb(radiance);
			  outColor.a = 1.0;
			}
			)"""")
		.addUniform(PHONG_CAMERA_LOOKAT_UNIFORM_NAME, eUniformSemantic::viewMatrix)
		.addUniform(PHONG_CAMERA_PROJECTION_UNIFORM_NAME, eUniformSemantic::projectionMatrix)
		.addUniform(PHONG_MESH_TRANSFORM_UNIFORM_NAME, eUniformSemantic::modelMatrix)
		.addUniform(PHONG_INV_MESH_TRANSFORM_UNIFORM_NAME, eUniformSemantic::normalMatrix)
		.addUniform(PHONG_AMBIENT_COLOR_UNIFORM_NAME, eUniformSemantic::ambientColorRGBA)
		.addUniform(PHONG_DIFFUSE_COLOR_UNIFORM_NAME, eUniformSemantic::diffuseColorRGBA)
		.addUniform(PHONG_SPECULAR_COLOR_UNIFORM_NAME, eUniformSemantic::specularColorRGBA)
		.addUniform(PHONG_SHININESS_UNIFORM_NAME, eUniformSemantic::shininess)
		.addUniform(PHONG_LIGHT_COLOR_UNIFORM_NAME, eUniformSemantic::lightColor)
		.addUniform(PHONG_LIGHT_DIRECTION_UNIFORM_NAME, eUniformSemantic::lightDirection)
		.addUniform(PHONG_CAMERA_POSITION_UNIFORM_NAME, eUniformSemantic::cameraPosition);

	return &x_shaderCode;
}

const GlProgramCode* GlModelResourceManager::getWireframeShaderCode()
{
	static GlProgramCode x_shaderCode = GlProgramCode(
		"wireframe line shader",
		// vertex shader
		R""""(
		#version 410 
		uniform mat4 mvpMatrix; 
		layout(location = 0) in vec3 in_position; 
		void main() 
		{ 
			gl_Position = mvpMatrix * vec4(in_position.xyz, 1); 
		}
		)"""",
		//fragment shader
		R""""(
		#version 410 core
		uniform vec4 diffuseColor; 
		out vec4 out_FragColor;
		void main()
		{
			out_FragColor = diffuseColor;
		}
		)"""")
		.addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix)
		.addUniform("diffuseColor", eUniformSemantic::diffuseColorRGBA);

	return &x_shaderCode;
}