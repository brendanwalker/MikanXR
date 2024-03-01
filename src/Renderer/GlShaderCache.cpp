#include "GlMaterial.h"
#include "GlShaderCache.h"
#include "GlProgram.h"
#include "GlProgramConfig.h"
#include "MaterialAssetReference.h"
#include "Logger.h"

namespace InternalShaders
{
	bool registerInternalShaders(GlShaderCache& shaderCache);
}

bool GlShaderCache::startup()
{
	return InternalShaders::registerInternalShaders(*this);
}

void GlShaderCache::shutdown()
{
	m_programCache.clear();
}

GlMaterialPtr GlShaderCache::loadMaterialAssetReference(MaterialAssetReferencePtr materialAssetRef)
{
	GlMaterialPtr material;

	if (materialAssetRef && materialAssetRef->isValid())
	{
		auto shaderFilePath = materialAssetRef->getAssetPath();

		GlProgramConfig programConfig;
		if (programConfig.load(shaderFilePath))
		{
			GlProgramCode programCode;
			if (programConfig.loadGlProgramCode(&programCode))
			{
				material = registerMaterial(programCode);
			}
			else
			{
				MIKAN_LOG_ERROR("GlShaderCache::loadMaterialAssetReference")
					<< "Failed material program code: " << shaderFilePath;
			}
		}
		else
		{
			MIKAN_LOG_ERROR("GlShaderCache::loadMaterialAssetReference")
				<< "Failed material config load: " << shaderFilePath;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("GlShaderCache::loadMaterialAssetReference") << "Invalid material asset ref";
	}

	return material;
}

GlMaterialPtr GlShaderCache::registerMaterial(const GlProgramCode& code)
{
	const std::string materialName= code.getProgramName();

	auto it = m_materialCache.find(materialName);
	if (it != m_materialCache.end())
	{
		MIKAN_LOG_ERROR("GlShaderCache::registerMaterial") << "Material already exists: " << materialName;
		return GlMaterialPtr();
	}

	GlProgramPtr program = fetchCompiledGlProgram(&code);
	if (program)
	{
		auto material= std::make_shared<GlMaterial>(materialName, program);

		m_materialCache.insert({materialName, material});
		return material;
	}
	else
	{
		MIKAN_LOG_ERROR("GlShaderCache::registerMaterial") << "Failed to compile material: " << materialName;
		return GlMaterialPtr();
	}
}

GlMaterialConstPtr GlShaderCache::getMaterialByName(const std::string& name)
{
	auto it = m_materialCache.find(name);
	if (it != m_materialCache.end())
	{
		return it->second;
	}

	return GlMaterialPtr();
}

GlProgramPtr GlShaderCache::fetchCompiledGlProgram(
	const GlProgramCode* code)
{
	auto it = m_programCache.find(code->getProgramName());
	if (it != m_programCache.end())
	{
		GlProgramPtr existingProgram= it->second;

		if (existingProgram->getProgramCode().getCodeHash() == code->getCodeHash())
		{
			// Found a compiled version of the code
			return existingProgram;
		}
		else
		{
			// Old compiled program is stale so delete it
			m_programCache.erase(it);
		}
	}

	// (Re)compile program and add it to the cache
	GlProgramPtr program = std::make_shared<GlProgram>(*code);
	if (program->compileProgram())
	{
		m_programCache[code->getProgramName()] = program;
		return program;
	}
	else
	{
		// Clean up the program if it failed to compile
		return nullptr;
	}
}

namespace InternalShaders
{
	const GlProgramCode* getPTTexturedFullScreenQuad()
	{
		static GlProgramCode x_shaderCode = GlProgramCode(
			INTERNAL_MATERIAL_PT_FULLSCREEN_TEXTURE,
			// vertex shader
			R""""(
			#version 330 core
			layout (location = 0) in vec2 aPos;
			layout (location = 1) in vec2 aTexCoords;

			out vec2 TexCoords;

			void main()
			{
				TexCoords = aTexCoords;
				gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
			}
			)"""",
			//fragment shader
			R""""(
			#version 330 core
			out vec4 FragColor;

			in vec2 TexCoords;

			uniform sampler2D rgbTexture;

			void main()
			{
				vec3 col = texture(rgbTexture, TexCoords).rgb;
				FragColor = vec4(col, 1.0);
			} 
			)"""")
			.addUniform("rgbTexture", eUniformSemantic::texture0);

		return &x_shaderCode;
	}

	const GlProgramCode* getPNTPhongShaderCode()
	{
		// https://www.gsn-lib.org/docs/nodes/ShaderPluginNode.php
		static GlProgramCode x_shaderCode = GlProgramCode(
			INTERNAL_MATERIAL_PNT_BLINN_PHONG,
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
			.addUniform("cameraLookAt", eUniformSemantic::viewMatrix)
			.addUniform("cameraProjection", eUniformSemantic::projectionMatrix)
			.addUniform("meshTransform", eUniformSemantic::modelMatrix)
			.addUniform("meshTransformTransposedInverse", eUniformSemantic::normalMatrix)
			.addUniform("ambientColor", eUniformSemantic::ambientColorRGBA)
			.addUniform("diffuseColor", eUniformSemantic::diffuseColorRGBA)
			.addUniform("specularColor", eUniformSemantic::specularColorRGBA)
			.addUniform("shininess", eUniformSemantic::shininess)
			.addUniform("lightColor", eUniformSemantic::lightColor)
			.addUniform("lightDirection", eUniformSemantic::lightDirection)
			.addUniform("cameraPosition", eUniformSemantic::cameraPosition);

		return &x_shaderCode;
	}

	const GlProgramCode* getPWireframeShaderCode()
	{
		static GlProgramCode x_shaderCode = GlProgramCode(
			INTERNAL_MATERIAL_P_WIREFRAME,
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

	const GlProgramCode* getPSolidColorShaderCode()
	{
		static GlProgramCode x_shaderCode = GlProgramCode(
			INTERNAL_MATERIAL_P_SOLID_COLOR,
			// vertex shader
			R""""(
			#version 330 core
			layout (location = 0) in vec3 aPos;

			uniform mat4 mvpMatrix;
			uniform vec4 diffuseColor;

			void main()
			{
				gl_Position = mvpMatrix * vec4(aPos, 1.0);
			}
			)"""",
			//fragment shader
			R""""(
			#version 330 core
			out vec4 FragColor;

			void main()
			{    
				FragColor = vec4(1, 1, 1, 1);
			}
			)"""")
			.addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix)
			.addUniform("diffuseColor", eUniformSemantic::diffuseColorRGBA);

		return &x_shaderCode;
	}

	const GlProgramCode* getPTTexturedShaderCode()
	{
		static GlProgramCode x_shaderCode = GlProgramCode(
			INTERNAL_MATERIAL_PT_TEXTURED,
			// vertex shader
			R""""(
			#version 330 core
			layout (location = 0) in vec2 aPos;
			layout (location = 1) in vec2 aTexCoords;

			uniform mat4 mvpMatrix;

			out vec2 TexCoords;

			void main()
			{
				TexCoords = aTexCoords;
				gl_Position = mvpMatrix * vec4(aPos, 1.0);
			}  
			)"""",
			//fragment shader
			R""""(
			#version 330 core
			out vec4 FragColor;

			in vec2 TexCoords;

			uniform sampler2D rgbTexture;

			void main()
			{
				vec3 col = texture(rgbTexture, TexCoords).rgb;
				FragColor = vec4(col, 1.0);
			} 
			)"""")
			.addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix)
			.addUniform("rgbTexture", eUniformSemantic::diffuseTexture);

		return &x_shaderCode;
	}

	const GlProgramCode* getPNTTexturedColoredShaderCode()
	{
		static GlProgramCode x_shaderCode = GlProgramCode(
			INTERNAL_MATERIAL_PNT_TEXTURED_COLORED,
			// vertex shader
			R""""(
			#version 410 
			uniform mat4 matrix; 
			layout(location = 0) in vec4 position; 
			layout(location = 1) in vec3 v3NormalIn; 
			layout(location = 2) in vec2 v2TexCoordsIn; 
			out vec2 v2TexCoord; 
			void main() 
			{ 
				v2TexCoord = v2TexCoordsIn; 
				gl_Position = matrix * vec4(position.xyz, 1); 
			}
			)"""",
			//fragment shader
			R""""(
			#version 410 core
			uniform sampler2D diffuse;
			uniform vec4 modelColor;
			in vec2 v2TexCoord;
			out vec4 outputColor;
			void main()
			{
				outputColor = texture(diffuse, v2TexCoord) * modelColor;
			}
			)"""")
			.addUniform("matrix", eUniformSemantic::modelViewProjectionMatrix)
			.addUniform("diffuse", eUniformSemantic::diffuseTexture)
			.addUniform("modelColor", eUniformSemantic::diffuseColorRGBA);

		return &x_shaderCode;
	}

	bool registerInternalShaders(GlShaderCache& shaderCache)
	{
		std::vector<const GlProgramCode*> internalShaders = {
			getPTTexturedFullScreenQuad(),
			getPNTPhongShaderCode(),
			getPWireframeShaderCode(),
			getPSolidColorShaderCode(),
			getPTTexturedShaderCode(),
			getPNTTexturedColoredShaderCode()
		};

		bool bSuccess = true;
		for (const GlProgramCode* code : internalShaders)
		{
			if (shaderCache.registerMaterial(*code))
			{
				MIKAN_LOG_ERROR("InternalShaders::registerInternalShaders()") <<
					"Failed to compile " << code->getProgramName();
				bSuccess = false;
			}
		}

		return bSuccess;
	}
};

