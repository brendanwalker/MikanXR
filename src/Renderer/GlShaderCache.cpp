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

			void main()
			{
				gl_Position = mvpMatrix * vec4(aPos, 1.0);
			}
			)"""",
			//fragment shader
			R""""(
			#version 330 core
			out vec4 FragColor;

			uniform vec4 diffuseColor;

			void main()
			{    
				FragColor = diffuseColor;
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
			layout (location = 0) in vec3 aPos;
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
			INTERNAL_MATERIAL_PNT_TEXTURED_LIT_COLORED,
			// vertex shader
			R""""(
			#version 410 
			uniform mat4 matrix; 
			layout(location = 0) in vec3 aPos; 
			layout(location = 1) in vec3 v3NormalIn; 
			layout(location = 2) in vec2 v2TexCoordsIn; 

			out vec3 Normal; // Normal for the fragment shader
			out vec2 TexCoords; // Texture coordinate for the fragment shader
			out vec3 FragPos; // Fragment position in world space

			uniform mat4 model; // Model matrix
			uniform mat4 view; // View matrix
			uniform mat4 projection; // Projection matrix

			void main() 
			{ 
				FragPos = vec3(model * vec4(aPos, 1.0)); // Position in world space
				Normal = mat3(transpose(inverse(model))) * v3NormalIn; // Transform normal to world space
				TexCoords = v2TexCoordsIn;
    
				gl_Position = projection * view * vec4(FragPos, 1.0); // Calculate the final position

			}
			)"""",
			//fragment shader
			R""""(
			#version 410 core

			in vec3 Normal; // Normal from the vertex shader
			in vec2 TexCoords; // Texture coordinate from the vertex shader
			in vec3 FragPos; // Position in world space from the vertex shader

			out vec4 FragColor;

			uniform sampler2D diffuse_tex;
			uniform vec3 lightDir; // The direction of the light
			uniform vec3 lightColor; // The color of the light
			uniform vec4 modelColor;

			void main()
			{
				// Ambient
				float ambientStrength = 0.1;
				vec3 ambient = ambientStrength * lightColor;
    
				// Diffuse
				vec3 norm = normalize(Normal);
				vec3 lightDirNormalized = normalize(-lightDir);
				float diff = max(dot(norm, lightDirNormalized), 0.0);
				vec3 diffuse = diff * lightColor;
    
				// Combine the two diffuse and ambient
				vec4 lighting = vec4(ambient + diffuse, 1.0);

				FragColor = texture(diffuse_tex, TexCoords) * lighting * modelColor;
			}
			)"""")
			.addUniform("model", eUniformSemantic::modelMatrix)
			.addUniform("view", eUniformSemantic::viewMatrix)
			.addUniform("projection", eUniformSemantic::projectionMatrix)
			.addUniform("diffuse_tex", eUniformSemantic::diffuseTexture)
			.addUniform("modelColor", eUniformSemantic::diffuseColorRGBA)
			.addUniform("lightDir", eUniformSemantic::lightDirection)
			.addUniform("lightColor", eUniformSemantic::lightColor);

		return &x_shaderCode;
	}

	bool registerInternalShaders(GlShaderCache& shaderCache)
	{
		std::vector<const GlProgramCode*> internalShaders = {
			getPTTexturedFullScreenQuad(),
			getPWireframeShaderCode(),
			getPSolidColorShaderCode(),
			getPTTexturedShaderCode(),
			getPNTTexturedColoredShaderCode()
		};

		bool bSuccess = true;
		for (const GlProgramCode* code : internalShaders)
		{
			if (!shaderCache.registerMaterial(*code))
			{
				MIKAN_LOG_ERROR("InternalShaders::registerInternalShaders()") <<
					"Failed to compile " << code->getProgramName();
				bSuccess = false;
			}
		}

		return bSuccess;
	}
};

