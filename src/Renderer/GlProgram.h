#pragma once


#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float4.hpp"

#include <map>
#include <string>
#include <vector>

#include <stdint.h>

enum class eUniformSemantic : int
{
	INVALID = -1,

	transformMatrix,
	modelViewProjectionMatrix,
	diffuseColorRGBA,
	diffuseColorRGB,
	screenPosition,
	floatConstant0,
	floatConstant1,
	floatConstant2,
	floatConstant3,
	texture0,
	texture1,
	texture2,
	texture3,
	texture4,
	texture5,
	texture6,
	texture7,
	texture8,
	texture9,
	texture10,
	texture11,
	texture12,
	texture13,
	texture14,
	texture15,
	texture16,
	texture17,
	texture18,
	texture19,
	texture20,
	texture21,
	texture22,
	texture23,
	texture24,
	texture25,
	texture26,
	texture27,
	texture28,
	texture29,
	texture30,
	texture31,

	COUNT
};
extern const std::string* k_UniformSemanticName;

class GlProgramCode
{
public:
	struct Uniform
	{
		std::string name;
		eUniformSemantic semantic;
	};

	GlProgramCode() = default;
	GlProgramCode(
		const std::string &filename, 
		const std::string& vertexCode, 
		const std::string& fragmentCode);

	bool loadFromConfig(const class GlProgramConfig* config);

	const std::string& getFilename() const { return m_filename; }
	inline const char* getVertexShaderCode() const { return m_vertexShaderCode.c_str(); }
	inline const char* getFragmentShaderCode() const { return m_framementShaderCode.c_str(); }
	inline size_t getCodeHash() const { return m_shaderCodeHash; }

	inline const std::vector<Uniform>& getUniformList() const { return m_uniformList; }
	inline GlProgramCode& addUniform(const std::string& name, eUniformSemantic semantic)
	{
		m_uniformList.push_back({ name, semantic });
		return *this;
	}

	inline bool hasCode() const 
	{ 
		return m_vertexShaderCode.size() > 0 && m_framementShaderCode.size() > 0;
	}

	inline bool operator == (const GlProgramCode& other) const
	{ 
		return m_shaderCodeHash == other.m_shaderCodeHash;
	}

	inline bool operator != (const GlProgramCode& other) const
	{
		return m_shaderCodeHash != other.m_shaderCodeHash;
	}

protected:
	std::string m_filename;
	std::string m_vertexShaderCode;
	std::string m_framementShaderCode;
	std::vector<Uniform> m_uniformList;
	size_t m_shaderCodeHash;
};

class GlProgram
{
public:

	GlProgram() = default;
	GlProgram(const GlProgramCode &shaderCode);
	virtual ~GlProgram();

	bool setMatrix4x4Uniform(const eUniformSemantic semantic, const glm::mat4& mat);
	bool setFloatUniform(const eUniformSemantic semantic, const float value);
	bool setVector2Uniform(const eUniformSemantic semantic, const glm::vec2& vec);
	bool setVector3Uniform(const eUniformSemantic semantic, const glm::vec3& vec);
	bool setVector4Uniform(const eUniformSemantic semantic, const glm::vec4& vec);
	bool setTextureUniform(const eUniformSemantic semantic);

	bool createProgram();
	void deleteProgram();

	bool bindProgram() const;
	void unbindProgram() const;

protected:
	struct Uniform
	{
		std::string name;
		eUniformSemantic semantic;
		int locationId;
	};

	GlProgramCode m_code;
	uint32_t m_programID = 0;
	std::map<eUniformSemantic, Uniform> m_uniformLocationMap;
};