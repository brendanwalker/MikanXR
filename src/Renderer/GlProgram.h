#pragma once


#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float4.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <stdint.h>

#include "GlProgramConstants.h"

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
		const std::string& programName, 
		const std::string& vertexCode, 
		const std::string& fragmentCode);

	bool loadFromConfigData(
		const std::string& shaderConfigPath,
		const std::filesystem::path& vertexShaderFileName,
		const std::filesystem::path& fragmentShaderFileName,
		const std::map<std::string, std::string>& uniforms);

	const std::string& getProgramName() const { return m_programName; }
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
	std::string m_programName;
	std::string m_vertexShaderCode;
	std::string m_framementShaderCode;
	std::vector<Uniform> m_uniformList;
	size_t m_shaderCodeHash;
};

struct GlProgramUniform
{
	eUniformSemantic semantic;
	int locationId;
};
typedef std::map<std::string, GlProgramUniform> GlProgramUniformMap;
typedef std::map<std::string, GlProgramUniform>::const_iterator GlProgramUniformIter;

class GlProgram
{
public:

	GlProgram() = default;
	GlProgram(const GlProgramCode &shaderCode);
	virtual ~GlProgram();

	inline const GlProgramCode& getProgramCode() const { return m_code; }
	static eUniformDataType getUniformSemanticDataType(eUniformSemantic semantic);
	bool getUniformSemantic(const std::string uniformName, eUniformSemantic& outSemantic) const;
	bool getUniformDataType(const std::string uniformName, eUniformDataType& outDataType) const;
	static bool getTextureUniformUnit(eUniformSemantic semantic, int& outTextureUnit);
	bool getUniformTextureUnit(const std::string uniformName, int& outTextureUnit) const;
	GlProgramUniformIter getUniformBegin() const { return m_uniformLocationMap.begin(); }
	GlProgramUniformIter getUniformEnd() const { return m_uniformLocationMap.end(); }
	bool getFirstUniformNameOfSemantic(eUniformSemantic semantic, std::string& outUniformName) const;

	bool setMatrix4x4Uniform(const std::string uniformName, const glm::mat4& mat);
	bool setFloatUniform(const std::string uniformName, const float value);
	bool setVector2Uniform(const std::string uniformName, const glm::vec2& vec);
	bool setVector3Uniform(const std::string uniformName, const glm::vec3& vec);
	bool setVector4Uniform(const std::string uniformName, const glm::vec4& vec);
	bool setTextureUniform(const std::string uniformName);

	bool createProgram();
	void deleteProgram();

	bool bindProgram() const;
	void unbindProgram() const;

protected:
	GlProgramCode m_code;
	uint32_t m_programID = 0;
	GlProgramUniformMap m_uniformLocationMap;
};
typedef std::shared_ptr<GlProgram> GlProgramPtr;