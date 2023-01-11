#pragma once

#include "CommonConfig.h"
#include <filesystem>
#include <string>
#include <map>

struct GlProgramConfig : public CommonConfig
{
	GlProgramConfig(const std::string &program_name);

	const configuru::Config writeToJSON() const;
	void readFromJSON(const configuru::Config& pt);

	std::string materialName;
	std::filesystem::path vertexShaderPath;
	std::filesystem::path fragmentShaderPath;
	std::map<std::string, std::string> uniforms;
};