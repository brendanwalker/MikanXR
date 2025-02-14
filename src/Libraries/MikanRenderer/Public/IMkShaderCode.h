#pragma once

#include "MkRendererFwd.h"
#include "MkRendererExport.h"
#include "MkShaderConstants.h"
#include "MkVertexConstants.h"

#include <filesystem>
#include <string>
#include <vector>

class IMkShaderCode
{
public:
	struct Uniform
	{
		std::string name;
		eUniformSemantic semantic;
	};

	virtual const std::string& getProgramName() const = 0;
	virtual void setProgramName(const std::string& inName) = 0;

	virtual const char* getVertexShaderCode() const = 0;
	virtual const char* getFragmentShaderCode() const = 0;

	virtual const std::filesystem::path& getVertexShaderFilePath() const = 0;
	virtual void setVertexShaderFilePath(const std::filesystem::path& path) = 0;

	virtual const std::filesystem::path& getFragmentShaderFilePath() const = 0;
	virtual void setFragmentShaderFilePath(const std::filesystem::path& path) = 0;

	virtual size_t getCodeHash() const = 0;

	virtual const std::vector<IMkVertexAttributeConstPtr>& getVertexAttributes() const = 0;
	virtual void addVertexAttribute(
		const std::string& name, 
		eVertexDataType dataType, 
		eVertexSemantic semantic= eVertexSemantic::generic,
		bool isNormalized= false) = 0;

	virtual const std::vector<Uniform>& getUniformList() const = 0;
	virtual void addUniform(const std::string& name, eUniformSemantic semantic) = 0;

	virtual bool hasCode() const = 0;
	virtual bool operator == (const IMkShaderCode& other) const = 0;
	virtual bool operator != (const IMkShaderCode& other) const = 0;
};

MIKAN_RENDERER_FUNC(IMkShaderCodePtr) createIMkShaderCode();
MIKAN_RENDERER_FUNC(IMkShaderCodePtr) createIMkShaderCode(const std::string& programName);
MIKAN_RENDERER_FUNC(IMkShaderCodePtr) createIMkShaderCode(
	const std::string& programName,
	const std::string& vertexCode,
	const std::string& fragmentCode);