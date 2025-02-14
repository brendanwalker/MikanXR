#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"
#include "MkShaderConstants.h"
#include "IMkVertexDefinition.h"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/vector_int2_sized.hpp"
#include "glm/ext/vector_int3_sized.hpp"
#include "glm/ext/vector_int4_sized.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <stdint.h>

struct MkShaderUniform
{
	eUniformSemantic semantic;
	int locationId;
};
using MkShaderUniformMap= std::map<std::string, MkShaderUniform>;
using MkShaderUniformIter= std::map<std::string, MkShaderUniform>::const_iterator;

using MkUniformNameTextureUnitMap= std::map<std::string, int>;
using MkUniformNameTextureUnitMapIter= std::map<std::string, int>::const_iterator;

class IMkShader
{
public:
	virtual ~IMkShader() {}

	virtual IMkVertexDefinitionConstPtr getVertexDefinition() const = 0;
	virtual IMkShaderCodeConstPtr getProgramCode() const = 0;
	virtual IMkShaderCodePtr getProgramCodeMutable() = 0;
	virtual bool getUniformSemantic(const std::string uniformName, eUniformSemantic& outSemantic) const = 0;
	virtual bool getUniformDataType(const std::string uniformName, eUniformDataType& outDataType) const = 0;
	virtual std::vector<std::string> getUniformNamesOfDataType(const eUniformDataType dataType) const = 0;
	virtual bool getFirstTextureUnitOfSemantic(eUniformSemantic semantic, int& outTextureUnit) const = 0;
	virtual bool getUniformTextureUnit(const std::string uniformName, int& outTextureUnit) const = 0;
	virtual MkShaderUniformIter getUniformBegin() const = 0;
	virtual MkShaderUniformIter getUniformEnd() const = 0;
	virtual bool getFirstUniformNameOfSemantic(eUniformSemantic semantic, std::string& outUniformName) const = 0;

	virtual bool setMatrix4x4Uniform(const std::string uniformName, const glm::mat4& mat) = 0;
	virtual bool setIntUniform(const std::string uniformName, const int value) = 0;
	virtual bool setInt2Uniform(const std::string uniformName, const glm::ivec2& vec) = 0;
	virtual bool setInt3Uniform(const std::string uniformName, const glm::ivec3& vec) = 0;
	virtual bool setInt4Uniform(const std::string uniformName, const glm::ivec4& vec) = 0;
	virtual bool setFloatUniform(const std::string uniformName, const float value) = 0;
	virtual bool setVector2Uniform(const std::string uniformName, const glm::vec2& vec) = 0;
	virtual bool setVector3Uniform(const std::string uniformName, const glm::vec3& vec) = 0;
	virtual bool setVector4Uniform(const std::string uniformName, const glm::vec4& vec) = 0;
	virtual bool setTextureUniform(const std::string uniformName) = 0;

	virtual bool compileProgram() = 0;
	virtual bool isProgramCompiled() const = 0;
	virtual uint32_t getIMkShaderId() const = 0;
	virtual void deleteProgram() = 0;

	virtual bool bindProgram() const = 0;
	virtual void unbindProgram() const = 0;
};

MIKAN_RENDERER_FUNC(IMkShaderPtr) createIMkShader();
MIKAN_RENDERER_FUNC(IMkShaderPtr) createIMkShader(const std::string& programName);
MIKAN_RENDERER_FUNC(IMkShaderPtr) createIMkShader(IMkShaderCodeConstPtr shaderCode);