#pragma once

#include "CommonConfig.h"
#include <filesystem>
#include <string>
#include <map>

class GlProgramConfig : public CommonConfig
{
public:
	virtual configuru::Config writeToJSON() override;
	virtual void readFromJSON(const configuru::Config& pt) override;

	bool loadGlProgramCode(class GlProgramCode* outProgramCode);

	std::string materialName;
	std::filesystem::path vertexShaderPath;
	std::filesystem::path fragmentShaderPath;
	std::map<std::string, std::string> uniformSemanticMap;
};