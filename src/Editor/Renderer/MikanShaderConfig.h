#pragma once

#include "CommonConfig.h"
#include "MkVertexConstants.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

class MikanVertexAttributeConfig : public CommonConfig
{
public:
	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	std::string name;
	eVertexDataType dataType= eVertexDataType::INVALID;
	eVertexSemantic semantic= eVertexSemantic::INVALID;
};
using GlVertexAttributeConfigPtr = std::shared_ptr<MikanVertexAttributeConfig>;

class MikanShaderConfig : public CommonConfig
{
public:
	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	std::string materialName;
	std::filesystem::path vertexShaderPath;
	std::filesystem::path fragmentShaderPath;
	std::map<std::string, std::string> uniformSemanticMap;
	std::vector<GlVertexAttributeConfigPtr> vertexAttributes;
};