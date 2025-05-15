#include "IMkShaderCache.h"
#include "MkMaterial.h"
#include "IMkShader.h"
#include "IMkShaderCode.h"
#include "Logger.h"

namespace InternalShaders
{
	bool registerInternalShaders(IMkShaderCache* shaderCache);
}

class GlShaderCache : public IMkShaderCache
{
public:
	GlShaderCache() = delete;
	GlShaderCache(IMkWindow* ownerWindow)
		: m_ownerWindow(ownerWindow)
	{
	}
	virtual ~GlShaderCache()
	{
		shutdown();
	}

	virtual bool startup() override
	{
		return InternalShaders::registerInternalShaders(this);
	}

	virtual void shutdown() override
	{
		m_programCache.clear();
	}

	MkMaterialPtr GlShaderCache::registerMaterial(IMkShaderCodeConstPtr code)
	{
		const std::string materialName = code->getProgramName();

		auto it = m_materialCache.find(materialName);
		if (it != m_materialCache.end())
		{
			MIKAN_LOG_ERROR("GlShaderCache::registerMaterial") << "Material already exists: " << materialName;
			return MkMaterialPtr();
		}

		IMkShaderPtr program = fetchCompiledIMkShader(code);
		if (program)
		{
			auto material = std::make_shared<MkMaterial>(materialName, program);

			m_materialCache.insert({materialName, material});
			return material;
		}
		else
		{
			MIKAN_LOG_ERROR("GlShaderCache::registerMaterial") << "Failed to compile material: " << materialName;
			return MkMaterialPtr();
		}
	}

	MkMaterialConstPtr GlShaderCache::getMaterialByName(const std::string& name)
	{
		auto it = m_materialCache.find(name);
		if (it != m_materialCache.end())
		{
			return it->second;
		}

		return MkMaterialConstPtr();
	}

	IMkShaderPtr GlShaderCache::fetchCompiledIMkShader(
		IMkShaderCodeConstPtr code)
	{
		auto it = m_programCache.find(code->getProgramName());
		if (it != m_programCache.end())
		{
			IMkShaderPtr existingProgram = it->second;

			if (existingProgram->getProgramCode()->getCodeHash() == code->getCodeHash())
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
		IMkShaderPtr program = createIMkShader(code);
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

private:
	IMkWindow* m_ownerWindow;
	std::map<std::string, IMkShaderPtr> m_programCache;
	std::map<std::string, MkMaterialPtr> m_materialCache;
};

IMkShaderCachePtr CreateMkShaderCache(class IMkWindow* ownerWindow)
{
	return std::make_shared<GlShaderCache>(ownerWindow);
}

namespace InternalShaders
{
	IMkShaderCodeConstPtr getPTTexturedFullScreenRGBQuad()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;

		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
				INTERNAL_MATERIAL_PT_FULLSCREEN_RGB_TEXTURE,
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
				)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec2, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			x_shaderCode->addUniform("rgbTexture", eUniformSemantic::rgbTexture);
		}

		return x_shaderCode;
	}

	IMkShaderCodeConstPtr getPTTexturedFullScreenRGBAQuad()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;

		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
				INTERNAL_MATERIAL_PT_FULLSCREEN_RGBA_TEXTURE,
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

				uniform sampler2D rgbaTexture;

				void main()
				{
					FragColor = texture(rgbaTexture, TexCoords).rgba;
				} 
				)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec2, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			x_shaderCode->addUniform("rgbaTexture", eUniformSemantic::rgbaTexture);
		}

		return x_shaderCode;
	}

	IMkShaderCodeConstPtr getTextShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;

		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
				INTERNAL_MATERIAL_TEXT,
				// vertex shader
				R""""(
				#version 330 core
				layout (location = 0) in vec2 aPos;
				layout (location = 1) in vec2 aTexCoords;

				uniform vec2 screenSize;

				out vec2 TexCoords;

				void main()
				{
					TexCoords = aTexCoords;
					gl_Position = vec4(2.0*(aPos.x / screenSize.x) - 1.0, 1.0 - 2.0*(aPos.y / screenSize.y), 0.0, 1.0); 
				}
				)"""",
					//fragment shader
					R""""(
				#version 330 core
				out vec4 FragColor;

				in vec2 TexCoords;

				uniform sampler2D glyphTexture;

				void main()
				{
					vec4 col = texture(glyphTexture, TexCoords);
					FragColor = col;
				} 
			)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec2, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			x_shaderCode->addUniform("glyphTexture", eUniformSemantic::rgbaTexture);
			x_shaderCode->addUniform("screenSize", eUniformSemantic::screenSize);
		}

		return x_shaderCode;
	}

	IMkShaderCodeConstPtr getUnpackRGBALinearDepthTextureShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;

		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
				INTERNAL_MATERIAL_UNPACK_RGBA_DEPTH_TEXTURE,
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
				out float gl_FragDepth;

				in vec2 TexCoords;

				uniform sampler2D rgbaPackedDepthTexture;

				void main()
				{
					// Convert rgba8 packed linear depth to linear depth float
					vec4 rgba = texture(rgbaPackedDepthTexture, TexCoords).rgba;
					float linearDepth= dot( rgba, vec4(1.0, 1/255.0, 1/65025.0, 1/16581375.0) );

					// Output linear depth to color and non-linear depth to depth buffer
					FragColor = vec4(linearDepth, linearDepth, linearDepth, 1.0);
					gl_FragDepth = linearDepth;
				} 
				)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec2, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			x_shaderCode->addUniform("rgbaPackedDepthTexture", eUniformSemantic::rgbaTexture);
		}

		return x_shaderCode;
	}

	IMkShaderCodeConstPtr getPWireframeShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;

		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
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
				)"""");
			x_shaderCode->addVertexAttribute("in_position", eVertexDataType::datatype_vec3, eVertexSemantic::position);
			x_shaderCode->addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix);
			x_shaderCode->addUniform("diffuseColor", eUniformSemantic::diffuseColorRGBA);
		}

		return x_shaderCode;
	}

	IMkShaderCodeConstPtr getPSolidColorShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;
		
		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
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
				)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec3, eVertexSemantic::position);
			x_shaderCode->addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix);
			x_shaderCode->addUniform("diffuseColor", eUniformSemantic::diffuseColorRGBA);
		}

		return x_shaderCode;
	}

	IMkShaderCodeConstPtr getPNTTexturedShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;
		
		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
				INTERNAL_MATERIAL_PNT_TEXTURED,
				// vertex shader
				R""""(
				#version 330 core
				layout (location = 0) in vec3 aPos;
				layout(location = 1) in vec3 v3NormalIn; 
				layout (location = 2) in vec2 aTexCoords;

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
					vec3 col = texture(rgbTexture, vec2(TexCoords.x, 1 - TexCoords.y)).rgb;
					FragColor = vec4(col, 1.0);
				} 
				)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec3, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("v3NormalIn", eVertexDataType::datatype_vec3, eVertexSemantic::normal);
			x_shaderCode->addVertexAttribute("aTexCoords", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			x_shaderCode->addUniform("mvpMatrix", eUniformSemantic::modelViewProjectionMatrix);
			x_shaderCode->addUniform("rgbTexture", eUniformSemantic::diffuseTexture);
		}

		return x_shaderCode;
	}

	IMkShaderCodeConstPtr getPNTTexturedColoredShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;
		
		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
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
				)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec3, eVertexSemantic::position);
			x_shaderCode->addVertexAttribute("v3NormalIn", eVertexDataType::datatype_vec3, eVertexSemantic::normal);
			x_shaderCode->addVertexAttribute("v2TexCoordsIn", eVertexDataType::datatype_vec2, eVertexSemantic::texCoord);
			x_shaderCode->addUniform("model", eUniformSemantic::modelMatrix);
			x_shaderCode->addUniform("view", eUniformSemantic::viewMatrix);
			x_shaderCode->addUniform("projection", eUniformSemantic::projectionMatrix);
			x_shaderCode->addUniform("diffuse_tex", eUniformSemantic::diffuseTexture);
			x_shaderCode->addUniform("modelColor", eUniformSemantic::diffuseColorRGBA);
			x_shaderCode->addUniform("lightDir", eUniformSemantic::lightDirection);
			x_shaderCode->addUniform("lightColor", eUniformSemantic::lightColorRGB);
		}

		return x_shaderCode;
	}

	IMkShaderCodeConstPtr getPLinearDepthShaderCode()
	{
		static IMkShaderCodePtr x_shaderCode = nullptr;
		
		if (x_shaderCode == nullptr)
		{
			x_shaderCode = createIMkShaderCode(
				INTERNAL_MATERIAL_P_LINEAR_DEPTH,
				// vertex shader
				R""""(
				#version 330 core
				layout (location = 0) in vec3 aPos;

				uniform mat4 modelMatrix;
				uniform mat4 viewMatrix;
				uniform mat4 projMatrix;

				out float depth;

				void main()
				{
					mat4 mvMatrix= viewMatrix * modelMatrix;
					mat4 mvpMatrix= projMatrix * mvMatrix;

					gl_Position = mvpMatrix * vec4(aPos, 1.0);
					depth = -(mvMatrix * vec4(aPos, 1.0)).z;
				} 
				)"""",
					//fragment shader
					R""""(
				#version 330 core

				uniform mat4 projMatrix;

				in float depth;

				out vec4 FragColor;
				out float gl_FragDepth;

				void main()
				{
					// Compute the zNear and zFar from projection matrix
					float A = projMatrix[2].z; 
					float B = projMatrix[3].z;
					float zNear = - B / (1.0 - A);
					float zFar  =   B / (1.0 + A);

					// Compute linear depth [0, 1]
					float linearDepth = clamp((depth - zNear)/(zFar - zNear), 0.0, 1.0);

					// Output linear depth to color and depth buffers
					FragColor = vec4(linearDepth, linearDepth, linearDepth, 1.0);
					gl_FragDepth = linearDepth;
				} 
				)"""");
			x_shaderCode->addVertexAttribute("aPos", eVertexDataType::datatype_vec3, eVertexSemantic::position);
			x_shaderCode->addUniform("modelMatrix", eUniformSemantic::modelMatrix);
			x_shaderCode->addUniform("viewMatrix", eUniformSemantic::viewMatrix);
			x_shaderCode->addUniform("projMatrix", eUniformSemantic::projectionMatrix);
		}

		return x_shaderCode;
	}

	bool registerInternalShaders(IMkShaderCache* shaderCache)
	{
		std::vector<IMkShaderCodeConstPtr> internalShaders = {
			getPTTexturedFullScreenRGBQuad(),
			getPTTexturedFullScreenRGBAQuad(),
			getTextShaderCode(),
			getUnpackRGBALinearDepthTextureShaderCode(),
			getPWireframeShaderCode(),
			getPSolidColorShaderCode(),
			getPNTTexturedShaderCode(),
			getPNTTexturedColoredShaderCode(),
			getPLinearDepthShaderCode(),
		};

		bool bSuccess = true;
		for (IMkShaderCodeConstPtr code : internalShaders)
		{
			if (!shaderCache->registerMaterial(code))
			{
				MIKAN_LOG_ERROR("InternalShaders::registerInternalShaders()") <<
					"Failed to compile " << code->getProgramName();
				bSuccess = false;
			}
		}

		return bSuccess;
	}
};

